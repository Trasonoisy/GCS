#include <QtTest/QtTest>

#include "Protocol/MavlinkFrame.h"

using namespace gcs::protocol;

class TestMavlinkMissionCodec : public QObject
{
    Q_OBJECT
private slots:
    void countRoundTrip();
    void requestIntRoundTrip();
    void requestListRoundTrip();
    void ackRoundTrip();
    void itemIntRoundTrip();
    void itemIntZeroAltitudeWorks();
    void countDecodesThroughBuildV2Frame();
};

void TestMavlinkMissionCodec::countRoundTrip()
{
    msg::MissionCount in;
    in.count            = 17;
    in.target_system    = 1;
    in.target_component = 1;

    const QByteArray payload = msg::encodeMissionCount(in);
    QCOMPARE(payload.size(), 4);

    msg::MissionCount out;
    QVERIFY(msg::decodeMissionCount(payload, out));
    QCOMPARE(out.count,            in.count);
    QCOMPARE(out.target_system,    in.target_system);
    QCOMPARE(out.target_component, in.target_component);
}

void TestMavlinkMissionCodec::requestIntRoundTrip()
{
    msg::MissionRequestInt in;
    in.seq              = 5;
    in.target_system    = 1;
    in.target_component = 1;

    const QByteArray payload = msg::encodeMissionRequestInt(in);
    QCOMPARE(payload.size(), 4);

    msg::MissionRequestInt out;
    QVERIFY(msg::decodeMissionRequestInt(payload, out));
    QCOMPARE(out.seq,              in.seq);
    QCOMPARE(out.target_system,    in.target_system);
    QCOMPARE(out.target_component, in.target_component);
}

void TestMavlinkMissionCodec::requestListRoundTrip()
{
    msg::MissionRequestList in;
    in.target_system    = 7;
    in.target_component = 1;

    const QByteArray payload = msg::encodeMissionRequestList(in);
    QCOMPARE(payload.size(), 2);

    msg::MissionRequestList out;
    QVERIFY(msg::decodeMissionRequestList(payload, out));
    QCOMPARE(out.target_system,    in.target_system);
    QCOMPARE(out.target_component, in.target_component);
}

void TestMavlinkMissionCodec::ackRoundTrip()
{
    msg::MissionAck in;
    in.target_system    = 1;
    in.target_component = 1;
    in.type             = 5; // MAV_MISSION_INVALID

    const QByteArray payload = msg::encodeMissionAck(in);
    QCOMPARE(payload.size(), 3);

    msg::MissionAck out;
    QVERIFY(msg::decodeMissionAck(payload, out));
    QCOMPARE(out.target_system,    in.target_system);
    QCOMPARE(out.target_component, in.target_component);
    QCOMPARE(out.type,             in.type);
}

void TestMavlinkMissionCodec::itemIntRoundTrip()
{
    msg::MissionItemInt in;
    in.param1   = 1.25f;
    in.param2   = 2.5f;
    in.param3   = 0.0f;
    in.param4   = 90.0f;
    in.x        = 210200000;   // 21.02 deg * 1e7
    in.y        = 1058000000;  // 105.80 deg * 1e7
    in.z        = 50.5f;
    in.seq      = 3;
    in.command  = 16; // MAV_CMD_NAV_WAYPOINT
    in.target_system    = 1;
    in.target_component = 1;
    in.frame            = 6; // GLOBAL_RELATIVE_ALT_INT
    in.current          = 0;
    in.autocontinue     = 1;
    in.mission_type     = 0;

    const QByteArray payload = msg::encodeMissionItemInt(in);
    QCOMPARE(payload.size(), 38);

    msg::MissionItemInt out;
    QVERIFY(msg::decodeMissionItemInt(payload, out));
    QCOMPARE(out.param1,           in.param1);
    QCOMPARE(out.param2,           in.param2);
    QCOMPARE(out.param4,           in.param4);
    QCOMPARE(out.x,                in.x);
    QCOMPARE(out.y,                in.y);
    QCOMPARE(out.z,                in.z);
    QCOMPARE(out.seq,              in.seq);
    QCOMPARE(out.command,          in.command);
    QCOMPARE(out.target_system,    in.target_system);
    QCOMPARE(out.target_component, in.target_component);
    QCOMPARE(out.frame,            in.frame);
    QCOMPARE(out.autocontinue,     in.autocontinue);
}

void TestMavlinkMissionCodec::itemIntZeroAltitudeWorks()
{
    msg::MissionItemInt in;
    in.z = 0.0f;
    in.seq = 0;
    in.command = 22;
    in.target_system = 1;
    in.target_component = 1;
    const QByteArray payload = msg::encodeMissionItemInt(in);
    msg::MissionItemInt out;
    QVERIFY(msg::decodeMissionItemInt(payload, out));
    QCOMPARE(out.z, 0.0f);
}

void TestMavlinkMissionCodec::countDecodesThroughBuildV2Frame()
{
    // Sanity: a mission frame wrapped by buildV2Frame() round-trips through
    // the MAVLink v2 parser and its payload decodes identically. This proves
    // the wire format is what MAVLinkProtocol will see in production.
    msg::MissionCount in;
    in.count            = 5;
    in.target_system    = 1;
    in.target_component = 1;
    const QByteArray payload = msg::encodeMissionCount(in);
    const QByteArray frame   = buildV2Frame(/*sysid*/1, /*compid*/1,
                                            msgid::MissionCount, payload);

    MavlinkV2Parser parser;
    DecodedFrame decoded;
    bool got = false;
    parser.feed(frame, [&](const DecodedFrame& f) { decoded = f; got = true; });
    QVERIFY(got);
    QCOMPARE(decoded.msgid, msgid::MissionCount);
    QCOMPARE(decoded.payload.size(), payload.size());

    msg::MissionCount out;
    QVERIFY(msg::decodeMissionCount(decoded.payload, out));
    QCOMPARE(out.count,            in.count);
    QCOMPARE(out.target_system,    in.target_system);
    QCOMPARE(out.target_component, in.target_component);
}

QTEST_MAIN(TestMavlinkMissionCodec)
#include "tst_mavlink_mission_codec.moc"
