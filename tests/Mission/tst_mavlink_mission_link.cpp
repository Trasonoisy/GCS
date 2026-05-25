#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QtTest>

#include "Comms/LinkInterface.h"
#include "Mission/MavlinkMissionLink.h"
#include "Mission/MissionDownloader.h"
#include "Mission/MissionPlan.h"
#include "Mission/MissionUploader.h"
#include "Protocol/MAVLinkProtocol.h"
#include "Protocol/MavlinkFrame.h"

using gcs::comms::LinkInterface;
using gcs::comms::LinkKind;
using gcs::mission::MavlinkMissionLink;
using gcs::mission::MissionDownloader;
using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::mission::MissionUploader;
using gcs::protocol::MAVLinkProtocol;
using gcs::protocol::buildV2Frame;
using gcs::protocol::msg::MissionAck;
using gcs::protocol::msg::MissionItemInt;
using gcs::protocol::msg::MissionCount;
using gcs::protocol::msg::MissionRequestInt;
using gcs::protocol::msg::MissionRequestList;
using namespace gcs::protocol::msg;

// A loopback LinkInterface: bytes written here become bytes "received" on
// the partner link. Lets us model a SITL autopilot at one end of a single
// transport.
class LoopLink : public LinkInterface
{
    Q_OBJECT
public:
    explicit LoopLink(QObject* parent = nullptr) : LinkInterface(parent) {}
    QString  name() const override { return "loop"; }
    bool     isConnected() const override { return true; }
    LinkKind kind() const override { return m_kind; }
    void setKind(LinkKind k) { m_kind = k; }

    void setPartner(LoopLink* p) { m_partner = p; }

public slots:
    void connectLink() override {}
    void disconnectLink() override {}
    void writeBytes(const QByteArray& bytes) override
    {
        if (!m_partner) return;
        // Hop through the event loop so the receiving side processes the
        // bytes in a clean call stack — mirrors MockMissionLink's pattern.
        LoopLink* p = m_partner;
        QTimer::singleShot(0, p, [p, bytes] {
            emit p->bytesReceived(p, bytes);
        });
    }
private:
    LoopLink* m_partner = nullptr;
    LinkKind  m_kind    = LinkKind::Udp;
};

class TestMavlinkMissionLink : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void uploadThreeWaypointsAcrossLoop();
    void downloadEmptyMission();
    void downloadThreeWaypoints();
    void uploadAckRejectFailsClean();
    void filtersByTargetSysid();
};

void TestMavlinkMissionLink::initTestCase()
{
    qRegisterMetaType<MissionCount>("gcs::protocol::msg::MissionCount");
    qRegisterMetaType<MissionRequestInt>("gcs::protocol::msg::MissionRequestInt");
    qRegisterMetaType<MissionItemInt>("gcs::protocol::msg::MissionItemInt");
    qRegisterMetaType<MissionAck>("gcs::protocol::msg::MissionAck");
    qRegisterMetaType<MissionRequestList>("gcs::protocol::msg::MissionRequestList");
}

namespace {
MissionPlan threeWaypoints()
{
    MissionPlan p;
    for (int i = 0; i < 3; ++i) {
        MissionItem it;
        it.seq          = i;
        it.command      = gcs::mission::cmd::NavWaypoint;
        it.frame        = gcs::mission::frame::GlobalRelativeAltInt;
        it.latitudeDeg  = 21.02 + i * 0.0001;
        it.longitudeDeg = 105.80 + i * 0.0001;
        it.altitudeM    = 50.0 + i;
        it.acceptanceRadiusM = 2.5;
        it.yawDeg       = qQNaN();
        p.items.append(it);
    }
    return p;
}
} // namespace

// ---- A minimal "SITL autopilot" reactor that runs on the other end of the
// loop. We don't reuse MockMissionLink because that talks IMissionLink, not
// MAVLink frames. This reactor only models what we need to drive the GCS-
// side state machine. -------------------------------------------------------
class FakeAutopilot : public QObject
{
    Q_OBJECT
public:
    FakeAutopilot(MAVLinkProtocol* proto, LoopLink* outLink,
                  int sysid = 1, int compid = 1, QObject* parent = nullptr)
        : QObject(parent), m_proto(proto), m_out(outLink),
          m_sysid(sysid), m_compid(compid)
    {
        connect(proto, &MAVLinkProtocol::missionCountReceived, this,
                &FakeAutopilot::onCount);
        connect(proto, &MAVLinkProtocol::missionItemIntReceived, this,
                &FakeAutopilot::onItem);
        connect(proto, &MAVLinkProtocol::missionRequestListReceived, this,
                &FakeAutopilot::onRequestList);
        connect(proto, &MAVLinkProtocol::missionRequestIntReceived, this,
                &FakeAutopilot::onRequestInt);
    }

    void setStoredItems(QList<MissionItem> items) { m_storedItems = std::move(items); }
    void setAckResultOverride(int result)         { m_ackOverride = result; }
    int  receivedCount()  const                   { return m_expectedCount; }
    int  receivedItems()  const                   { return m_receivedCount; }

private slots:
    void onCount(int /*sysid*/, int /*compid*/, const MissionCount& m)
    {
        // FakeAutopilot replies only to messages addressed to its own sysid;
        // the frame's outer sysid identifies the SENDER (GCS), so we filter
        // by the payload's target_system instead.
        if (m.target_system != m_sysid) return;
        m_expectedCount = m.count;
        m_receivedCount = 0;
        if (m_expectedCount == 0) {
            sendAck(0);
            return;
        }
        sendRequest(0);
    }
    void onItem(int /*sysid*/, int /*compid*/, const MissionItemInt& m)
    {
        if (m.target_system != m_sysid) return;
        ++m_receivedCount;
        if (m_receivedCount >= m_expectedCount) {
            sendAck(m_ackOverride >= 0 ? m_ackOverride : 0);
        } else {
            sendRequest(m_receivedCount);
        }
    }
    void onRequestList(int /*sysid*/, int /*compid*/, const MissionRequestList& m)
    {
        if (m.target_system != m_sysid) return;
        // Reply with our stored mission count.
        MissionCount c;
        c.count            = static_cast<uint16_t>(m_storedItems.size());
        c.target_system    = 255;
        c.target_component = 190;
        sendFrame(gcs::protocol::msgid::MissionCount,
                  gcs::protocol::msg::encodeMissionCount(c));
    }
    void onRequestInt(int /*sysid*/, int /*compid*/, const MissionRequestInt& m)
    {
        if (m.target_system != m_sysid) return;
        if (m.seq >= m_storedItems.size()) return;
        const auto& it = m_storedItems.at(m.seq);
        MissionItemInt out;
        out.param1   = static_cast<float>(it.holdTimeSec);
        out.param2   = static_cast<float>(it.acceptanceRadiusM);
        out.param4   = std::isnan(it.yawDeg) ? std::nanf("") : float(it.yawDeg);
        out.x        = qRound(it.latitudeDeg  * 1.0e7);
        out.y        = qRound(it.longitudeDeg * 1.0e7);
        out.z        = static_cast<float>(it.altitudeM);
        out.seq      = static_cast<uint16_t>(it.seq);
        out.command  = static_cast<uint16_t>(it.command);
        out.target_system    = 255;
        out.target_component = 190;
        out.frame        = static_cast<uint8_t>(it.frame);
        out.autocontinue = it.autocontinue ? 1 : 0;
        sendFrame(gcs::protocol::msgid::MissionItemInt,
                  gcs::protocol::msg::encodeMissionItemInt(out));
    }

private:
    void sendRequest(int seq)
    {
        MissionRequestInt r;
        r.seq              = static_cast<uint16_t>(seq);
        r.target_system    = 255;
        r.target_component = 190;
        sendFrame(gcs::protocol::msgid::MissionRequestInt,
                  gcs::protocol::msg::encodeMissionRequestInt(r));
    }
    void sendAck(int result)
    {
        MissionAck a;
        a.target_system    = 255;
        a.target_component = 190;
        a.type             = static_cast<uint8_t>(result);
        sendFrame(gcs::protocol::msgid::MissionAck,
                  gcs::protocol::msg::encodeMissionAck(a));
    }
    void sendFrame(int msgid, const QByteArray& payload)
    {
        const QByteArray frame = buildV2Frame(m_sysid, m_compid, msgid, payload);
        m_out->writeBytes(frame);
    }

    MAVLinkProtocol* m_proto;
    LoopLink* m_out;
    int m_sysid, m_compid;
    int m_expectedCount = 0;
    int m_receivedCount = 0;
    int m_ackOverride   = -1;
    QList<MissionItem> m_storedItems;
};

// ---------- Tests ----------

void TestMavlinkMissionLink::uploadThreeWaypointsAcrossLoop()
{
    // Wire two halves of a transport with their own MAVLinkProtocol.
    LoopLink gcsLink;
    LoopLink apLink;
    gcsLink.setPartner(&apLink);
    apLink.setPartner(&gcsLink);

    MAVLinkProtocol gcsProto;
    MAVLinkProtocol apProto;
    gcsProto.attachLink(&gcsLink);
    apProto.attachLink(&apLink);

    FakeAutopilot ap(&apProto, &apLink, /*sysid*/1, /*compid*/1);

    MavlinkMissionLink mlink(/*targetSys*/1, /*targetComp*/1, &gcsProto, &gcsLink);
    MissionUploader up(&mlink);
    up.setTimeoutMs(500);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(threeWaypoints()));
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), true);
    QCOMPARE(ap.receivedCount(), 3);
    QCOMPARE(ap.receivedItems(), 3);
}

void TestMavlinkMissionLink::downloadEmptyMission()
{
    LoopLink gcsLink, apLink;
    gcsLink.setPartner(&apLink); apLink.setPartner(&gcsLink);

    MAVLinkProtocol gcsProto, apProto;
    gcsProto.attachLink(&gcsLink);
    apProto.attachLink(&apLink);

    FakeAutopilot ap(&apProto, &apLink);
    ap.setStoredItems({}); // empty mission

    MavlinkMissionLink mlink(1, 1, &gcsProto, &gcsLink);
    MissionDownloader dn(&mlink);
    dn.setTimeoutMs(500);

    QSignalSpy done(&dn, &MissionDownloader::completed);
    QVERIFY(dn.start());
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), true);
    QCOMPARE(dn.result().items.size(), 0);
}

void TestMavlinkMissionLink::downloadThreeWaypoints()
{
    LoopLink gcsLink, apLink;
    gcsLink.setPartner(&apLink); apLink.setPartner(&gcsLink);

    MAVLinkProtocol gcsProto, apProto;
    gcsProto.attachLink(&gcsLink);
    apProto.attachLink(&apLink);

    FakeAutopilot ap(&apProto, &apLink);
    ap.setStoredItems(threeWaypoints().items);

    MavlinkMissionLink mlink(1, 1, &gcsProto, &gcsLink);
    MissionDownloader dn(&mlink);
    dn.setTimeoutMs(500);

    QSignalSpy done(&dn, &MissionDownloader::completed);
    QVERIFY(dn.start());
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), true);
    QCOMPARE(dn.result().items.size(), 3);
    // Verify lat/lon survived the int32-1e7 round-trip.
    const auto& it = dn.result().items.at(0);
    QVERIFY(std::abs(it.latitudeDeg - 21.02) < 1e-6);
    QVERIFY(std::abs(it.longitudeDeg - 105.80) < 1e-6);
}

void TestMavlinkMissionLink::uploadAckRejectFailsClean()
{
    LoopLink gcsLink, apLink;
    gcsLink.setPartner(&apLink); apLink.setPartner(&gcsLink);

    MAVLinkProtocol gcsProto, apProto;
    gcsProto.attachLink(&gcsLink);
    apProto.attachLink(&apLink);

    FakeAutopilot ap(&apProto, &apLink);
    ap.setAckResultOverride(/*MAV_MISSION_UNSUPPORTED*/3);

    MavlinkMissionLink mlink(1, 1, &gcsProto, &gcsLink);
    MissionUploader up(&mlink);
    up.setTimeoutMs(500);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(threeWaypoints()));
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), false);
    QVERIFY(done.first().at(1).toString().contains("ACK"));
}

void TestMavlinkMissionLink::filtersByTargetSysid()
{
    // The link is bound to target sysid 1; messages from sysid 7 must NOT
    // drive its state machine (otherwise multi-vehicle SITL would corrupt
    // uploads).
    LoopLink gcsLink, apLink;
    gcsLink.setPartner(&apLink); apLink.setPartner(&gcsLink);

    MAVLinkProtocol gcsProto, apProto;
    gcsProto.attachLink(&gcsLink);
    apProto.attachLink(&apLink);

    // FakeAutopilot pretends to be sysid 7, but the link targets sysid 1.
    FakeAutopilot ap(&apProto, &apLink, /*sysid*/7, /*compid*/1);

    MavlinkMissionLink mlink(/*targetSys*/1, /*targetComp*/1, &gcsProto, &gcsLink);
    MissionUploader up(&mlink);
    up.setTimeoutMs(80);
    up.setMaxRetries(1);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(threeWaypoints()));
    QVERIFY(done.wait(1500));
    QCOMPARE(done.first().at(0).toBool(), false);
    QVERIFY(done.first().at(1).toString().contains("timed out"));
}

QTEST_MAIN(TestMavlinkMissionLink)
#include "tst_mavlink_mission_link.moc"
