#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Comms/LinkInterface.h"
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
};

static QByteArray heartbeatPayload(uint8_t main, uint8_t sub)
{
    QByteArray p(9, 0);
    const quint32 cm = (quint32(main) << 16) | (quint32(sub) << 24);
    p[0] = char(cm & 0xFF);
    p[1] = char((cm >> 8) & 0xFF);
    p[2] = char((cm >> 16) & 0xFF);
    p[3] = char((cm >> 24) & 0xFF);
    p[4] = char(2);       // MAV_TYPE_QUADROTOR
    p[5] = char(12);      // MAV_AUTOPILOT_PX4
    p[6] = char(0x81);    // custom enabled | armed
    p[7] = char(4);       // ACTIVE
    p[8] = char(3);
    return p;
}

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
                                        heartbeatPayload(4, 4)));

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
                                        heartbeatPayload(4, 4))); // AUTO.Mission

    auto* veh = mgr.findBySysCompId(1, 1);
    QVERIFY(veh);
    const auto& st = veh->stateStore()->state();
    QCOMPARE(st.linkStatus, LinkStatus::Connected);
    QCOMPARE(st.flightMode, QStringLiteral("Mission"));
    QVERIFY(st.armed); // base mode 0x81 had ARMED flag
}

QTEST_MAIN(TestMessageRouter)
#include "tst_message_router.moc"
