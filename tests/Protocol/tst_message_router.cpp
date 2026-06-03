#include <QSignalSpy>
#include <QtTest/QtTest>
#include <cstring>

#include "Comms/LinkInterface.h"
#include "Firmware/ArduCopterFirmwarePlugin.h"
#include "Firmware/ArduPlaneFirmwarePlugin.h"
#include "Firmware/ArduRoverFirmwarePlugin.h"
#include "Firmware/ArduSubFirmwarePlugin.h"
#include "Firmware/FirmwarePluginManager.h"
#include "Firmware/MavType.h"
#include "Firmware/PX4FirmwarePlugin.h"
#include "Protocol/MAVLinkMessageRouter.h"
#include "Protocol/MAVLinkProtocol.h"
#include "Protocol/MavlinkFrame.h"
#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

using namespace gcs::protocol;
using gcs::vehicle::Vehicle;
using gcs::vehicle::MultiVehicleManager;
using gcs::vehicle::LinkStatus;

class TestMessageRouter : public QObject
{
    Q_OBJECT
private slots:
    void unknownHeartbeatSpawnsVehicle();
    void nonHeartbeatFromUnknownIsDropped();
    void heartbeatPromotesLinkAndDecodesMode();
    void ardupilotHeartbeatCreatesExpectedFirmwarePlugin_data();
    void ardupilotHeartbeatCreatesExpectedFirmwarePlugin();
    void serialTelemetryCreatesReadOnlyHardwareVehicleAndUpdatesState();
};

static QByteArray heartbeatPayload(uint8_t mavType, uint8_t autopilot,
                                   uint32_t customMode, uint8_t baseMode)
{
    QByteArray p(9, 0);
    p[0] = char(customMode & 0xFF);
    p[1] = char((customMode >> 8) & 0xFF);
    p[2] = char((customMode >> 16) & 0xFF);
    p[3] = char((customMode >> 24) & 0xFF);
    p[4] = char(mavType);
    p[5] = char(autopilot);
    p[6] = char(baseMode);
    p[7] = char(4);       // ACTIVE
    p[8] = char(3);
    return p;
}

static QByteArray px4HeartbeatPayload(uint8_t main, uint8_t sub)
{
    const quint32 customMode = (quint32(main) << 16) | (quint32(sub) << 24);
    return heartbeatPayload(/*MAV_TYPE_QUADROTOR*/2, gcs::firmware::autopilot::Px4,
                            customMode, /*custom enabled | armed*/0x81);
}

namespace {
class TestLink final : public gcs::comms::LinkInterface
{
    Q_OBJECT
public:
    explicit TestLink(gcs::comms::LinkKind kind, QObject* parent = nullptr)
        : gcs::comms::LinkInterface(parent), m_kind(kind)
    {
    }

    QString name() const override { return QStringLiteral("test-link"); }
    bool isConnected() const override { return true; }
    gcs::comms::LinkKind kind() const override { return m_kind; }

    void connectLink() override {}
    void disconnectLink() override {}
    void writeBytes(const QByteArray&) override {}

    void inject(const QByteArray& bytes) { emit bytesReceived(this, bytes); }

private:
    gcs::comms::LinkKind m_kind;
};

template <typename T>
bool isInstanceOf(QObject* obj)
{
    return qobject_cast<T*>(obj) != nullptr;
}

template <typename T>
void writeLE(QByteArray& p, int offset, T value)
{
    if (p.size() < offset + int(sizeof(T))) p.resize(offset + int(sizeof(T)));
    std::memcpy(p.data() + offset, &value, sizeof(T));
}
} // namespace

void TestMessageRouter::unknownHeartbeatSpawnsVehicle()
{
    MultiVehicleManager mgr;
    MAVLinkProtocol     proto;
    MAVLinkMessageRouter router;
    router.setMultiVehicleManager(&mgr);
    router.setProtocol(&proto);

    bool factoryRan = false;
    router.setVehicleFactory(
        [&](int sysid, int compid, int /*ap*/, int /*mavType*/,
            gcs::comms::LinkKind /*linkKind*/,
            gcs::comms::LinkInterface* /*sourceLink*/) -> Vehicle* {
            factoryRan = true;
            auto* fw = new gcs::firmware::PX4FirmwarePlugin(&mgr);
            auto* v  = new Vehicle(sysid, compid, fw, &mgr);
            mgr.addVehicle(v);
            return v;
        });

    QSignalSpy spawnSpy(&router, &MAVLinkMessageRouter::vehicleSpawned);
    proto.feedBytesForTest(buildV2Frame(1, 1, msgid::Heartbeat,
                                        px4HeartbeatPayload(4, 4)));

    QVERIFY(factoryRan);
    QCOMPARE(spawnSpy.count(), 1);
    QCOMPARE(mgr.vehicles().size(), 1);
    QVERIFY(mgr.findBySysCompId(1, 1) != nullptr);
}

void TestMessageRouter::nonHeartbeatFromUnknownIsDropped()
{
    MultiVehicleManager mgr;
    MAVLinkProtocol     proto;
    MAVLinkMessageRouter router;
    router.setMultiVehicleManager(&mgr);
    router.setProtocol(&proto);
    // No factory set — so even a heartbeat couldn't register; but here we
    // send an attitude message that should be dropped without any factory call.

    QByteArray attPayload(28, 0);
    proto.feedBytesForTest(buildV2Frame(9, 1, msgid::Attitude, attPayload));

    QCOMPARE(mgr.vehicles().size(), 0);
    QVERIFY(router.droppedMessageCount() >= 1);
    QCOMPARE(router.routedMessageCount(), 0);
}

void TestMessageRouter::heartbeatPromotesLinkAndDecodesMode()
{
    MultiVehicleManager mgr;
    MAVLinkProtocol     proto;
    MAVLinkMessageRouter router;
    router.setMultiVehicleManager(&mgr);
    router.setProtocol(&proto);
    router.setVehicleFactory(
        [&](int sysid, int compid, int, int, gcs::comms::LinkKind,
            gcs::comms::LinkInterface*) -> Vehicle* {
            auto* fw = new gcs::firmware::PX4FirmwarePlugin(&mgr);
            auto* v  = new Vehicle(sysid, compid, fw, &mgr);
            mgr.addVehicle(v);
            return v;
        });

    proto.feedBytesForTest(buildV2Frame(1, 1, msgid::Heartbeat,
                                        px4HeartbeatPayload(4, 4))); // AUTO.Mission

    auto* veh = mgr.findBySysCompId(1, 1);
    QVERIFY(veh);
    const auto& st = veh->stateStore()->state();
    QCOMPARE(st.linkStatus, LinkStatus::Connected);
    QCOMPARE(st.flightMode, QStringLiteral("Mission"));
    QVERIFY(st.armed); // base mode 0x81 had ARMED flag
}

void TestMessageRouter::ardupilotHeartbeatCreatesExpectedFirmwarePlugin_data()
{
    QTest::addColumn<int>("mavType");
    QTest::addColumn<int>("customMode");
    QTest::addColumn<QString>("vehicleType");
    QTest::addColumn<QString>("flightMode");

    QTest::newRow("copter") << 2  << 5  << QStringLiteral("Copter")
                            << QStringLiteral("Loiter");
    QTest::newRow("plane")  << 1  << 12 << QStringLiteral("Plane")
                            << QStringLiteral("Loiter");
    QTest::newRow("rover")  << 10 << 15 << QStringLiteral("Rover")
                            << QStringLiteral("Guided");
    QTest::newRow("sub")    << 12 << 19 << QStringLiteral("Sub")
                            << QStringLiteral("Manual");
}

void TestMessageRouter::ardupilotHeartbeatCreatesExpectedFirmwarePlugin()
{
    QFETCH(int, mavType);
    QFETCH(int, customMode);
    QFETCH(QString, vehicleType);
    QFETCH(QString, flightMode);

    MultiVehicleManager mgr;
    MAVLinkProtocol proto;
    MAVLinkMessageRouter router;
    router.setMultiVehicleManager(&mgr);
    router.setProtocol(&proto);
    router.setVehicleFactory(
        [&](int sysid, int compid, int autopilot, int type,
            gcs::comms::LinkKind linkKind,
            gcs::comms::LinkInterface*) -> Vehicle* {
            auto* fw = gcs::firmware::FirmwarePluginManager::createForHeartbeat(
                static_cast<uint8_t>(autopilot),
                static_cast<uint8_t>(type),
                &mgr);
            auto* v = new Vehicle(sysid, compid, fw, &mgr);
            v->stateStore()->setLinkKind(linkKind);
            mgr.addVehicle(v);
            return v;
        });

    proto.feedBytesForTest(buildV2Frame(1, 1, msgid::Heartbeat,
        heartbeatPayload(static_cast<uint8_t>(mavType),
                         gcs::firmware::autopilot::ArduPilotMega,
                         static_cast<uint32_t>(customMode),
                         /*custom enabled*/0x01)));

    auto* veh = mgr.findBySysCompId(1, 1);
    QVERIFY(veh);
    QCOMPARE(veh->stateStore()->state().autopilotType,
             QStringLiteral("ArduPilot"));
    QCOMPARE(veh->stateStore()->state().vehicleType, vehicleType);
    QCOMPARE(veh->stateStore()->state().flightMode, flightMode);

    auto* fw = veh->firmwarePlugin();
    QVERIFY(fw);
    if (vehicleType == QStringLiteral("Copter"))
        QVERIFY(isInstanceOf<gcs::firmware::ArduCopterFirmwarePlugin>(fw));
    else if (vehicleType == QStringLiteral("Plane"))
        QVERIFY(isInstanceOf<gcs::firmware::ArduPlaneFirmwarePlugin>(fw));
    else if (vehicleType == QStringLiteral("Rover"))
        QVERIFY(isInstanceOf<gcs::firmware::ArduRoverFirmwarePlugin>(fw));
    else if (vehicleType == QStringLiteral("Sub"))
        QVERIFY(isInstanceOf<gcs::firmware::ArduSubFirmwarePlugin>(fw));
}

void TestMessageRouter::serialTelemetryCreatesReadOnlyHardwareVehicleAndUpdatesState()
{
    MultiVehicleManager mgr;
    MAVLinkProtocol proto;
    MAVLinkMessageRouter router;
    TestLink serialLink(gcs::comms::LinkKind::Serial);

    router.setMultiVehicleManager(&mgr);
    router.setProtocol(&proto);
    proto.attachLink(&serialLink);

    gcs::comms::LinkKind factoryLinkKind = gcs::comms::LinkKind::Unknown;
    gcs::comms::LinkInterface* factorySourceLink = nullptr;
    router.setVehicleFactory(
        [&](int sysid, int compid, int autopilot, int type,
            gcs::comms::LinkKind linkKind,
            gcs::comms::LinkInterface* sourceLink) -> Vehicle* {
            factoryLinkKind = linkKind;
            factorySourceLink = sourceLink;
            auto* fw = gcs::firmware::FirmwarePluginManager::createForHeartbeat(
                static_cast<uint8_t>(autopilot),
                static_cast<uint8_t>(type),
                &mgr);
            auto* v = new Vehicle(sysid, compid, fw, &mgr);
            v->stateStore()->setLinkKind(linkKind);
            mgr.addVehicle(v);
            return v;
        });

    serialLink.inject(buildV2Frame(42, 1, msgid::Heartbeat,
                                   px4HeartbeatPayload(4, 4)));

    auto* veh = mgr.findBySysCompId(42, 1);
    QVERIFY(veh);
    QCOMPARE(factoryLinkKind, gcs::comms::LinkKind::Serial);
    QCOMPARE(factorySourceLink, &serialLink);
    QCOMPARE(veh->stateStore()->state().linkKind, gcs::comms::LinkKind::Serial);
    QCOMPARE(veh->stateStore()->state().linkStatus, LinkStatus::Connected);
    QCOMPARE(veh->stateStore()->state().autopilotType, QStringLiteral("PX4"));
    QCOMPARE(veh->stateStore()->state().flightMode, QStringLiteral("Mission"));

    QByteArray sysStatus(31, 0);
    writeLE<quint16>(sysStatus, 14, 12600);
    writeLE<qint8>(sysStatus, 30, 88);
    serialLink.inject(buildV2Frame(42, 1, msgid::SysStatus, sysStatus));

    QByteArray gps(30, 0);
    writeLE<quint8>(gps, 28, 3);
    writeLE<quint8>(gps, 29, 11);
    serialLink.inject(buildV2Frame(42, 1, msgid::GpsRawInt, gps));

    QByteArray attitude(28, 0);
    writeLE<float>(attitude, 4, 0.10f);
    writeLE<float>(attitude, 8, -0.20f);
    writeLE<float>(attitude, 12, 1.57f);
    serialLink.inject(buildV2Frame(42, 1, msgid::Attitude, attitude));

    QByteArray position(28, 0);
    writeLE<qint32>(position, 4, qint32(210285000));
    writeLE<qint32>(position, 8, qint32(1058048000));
    writeLE<qint32>(position, 16, qint32(45600));
    writeLE<quint16>(position, 26, quint16(8600));
    serialLink.inject(buildV2Frame(42, 1, msgid::GlobalPositionInt, position));

    QByteArray vfr(20, 0);
    writeLE<float>(vfr, 4, 6.25f);
    serialLink.inject(buildV2Frame(42, 1, msgid::VfrHud, vfr));

    const auto& s = veh->stateStore()->state();
    QCOMPARE(s.batteryVoltage, 12.6);
    QCOMPARE(s.batteryPercent, 88.0);
    QCOMPARE(s.gpsFixType, 3);
    QCOMPARE(s.satellitesVisible, 11);
    QCOMPARE(s.latitudeDeg, 21.0285);
    QCOMPARE(s.longitudeDeg, 105.8048);
    QCOMPARE(s.relativeAltitudeM, 45.6);
    QVERIFY(qAbs(s.rollDeg - 5.7296) < 0.01);
    QVERIFY(qAbs(s.pitchDeg + 11.4592) < 0.01);
    QCOMPARE(s.headingDeg, 86.0);
    QCOMPARE(s.groundSpeedMps, 6.25);
    QVERIFY(!veh->missionManager());
}

QTEST_MAIN(TestMessageRouter)
#include "tst_message_router.moc"
