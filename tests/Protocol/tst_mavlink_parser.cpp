#include <QtTest/QtTest>

#include "Protocol/MAVLinkProtocol.h"
#include "Protocol/MavlinkFrame.h"

using namespace gcs::protocol;

class TestMavlinkParser : public QObject
{
    Q_OBJECT
private slots:
    void parserEmitsFrame();
    void parserHandlesSplitChunks();
    void parserSkipsGarbageAndResyncs();
    void parserHandlesV1Frames();
    void protocolEmitsHeartbeat();
    void protocolEmitsGlobalPositionInt();
    void protocolEmitsAttitude();
};

static QByteArray heartbeatPayload(uint8_t main, uint8_t sub,
                                   uint8_t type = 2 /*MAV_TYPE_QUADROTOR*/,
                                   uint8_t autopilot = 12 /*PX4*/,
                                   uint8_t baseMode = 0x81 /*custom|armed*/,
                                   uint8_t systemStatus = 4 /*active*/)
{
    QByteArray p(9, 0);
    const quint32 cm = (quint32(main) << 16) | (quint32(sub) << 24);
    p[0] = char(cm & 0xFF);
    p[1] = char((cm >> 8) & 0xFF);
    p[2] = char((cm >> 16) & 0xFF);
    p[3] = char((cm >> 24) & 0xFF);
    p[4] = char(type);
    p[5] = char(autopilot);
    p[6] = char(baseMode);
    p[7] = char(systemStatus);
    p[8] = char(3); // mavlink_version
    return p;
}

void TestMavlinkParser::parserEmitsFrame()
{
    MavlinkV2Parser parser;
    int seen = 0;
    DecodedFrame got;

    const auto frame = buildV2Frame(1, 1, msgid::Heartbeat,
                                    heartbeatPayload(4, 4));
    parser.feed(frame, [&](const DecodedFrame& f) { got = f; ++seen; });

    QCOMPARE(seen, 1);
    QCOMPARE(got.sysid, 1);
    QCOMPARE(got.compid, 1);
    QCOMPARE(got.msgid, msgid::Heartbeat);
    QVERIFY(got.v2);
    QCOMPARE(got.payload.size(), 9);
}

void TestMavlinkParser::parserHandlesSplitChunks()
{
    MavlinkV2Parser parser;
    int seen = 0;

    const auto frame = buildV2Frame(1, 1, msgid::Heartbeat,
                                    heartbeatPayload(4, 4));
    // Feed in two halves.
    parser.feed(frame.left(5),  [&](const DecodedFrame&) { ++seen; });
    QCOMPARE(seen, 0);
    parser.feed(frame.mid(5),   [&](const DecodedFrame&) { ++seen; });
    QCOMPARE(seen, 1);
}

void TestMavlinkParser::parserSkipsGarbageAndResyncs()
{
    MavlinkV2Parser parser;
    int seen = 0;
    DecodedFrame got;

    const auto frame = buildV2Frame(1, 1, msgid::Heartbeat,
                                    heartbeatPayload(4, 4));
    QByteArray garbage("\xAA\xBB\xCC", 3);
    parser.feed(garbage + frame,
                [&](const DecodedFrame& f) { got = f; ++seen; });
    QCOMPARE(seen, 1);
    QCOMPARE(got.msgid, msgid::Heartbeat);
}

void TestMavlinkParser::parserHandlesV1Frames()
{
    MavlinkV2Parser parser;
    int seen = 0;
    DecodedFrame got;

    // Hand-roll a v1 HEARTBEAT frame: STX=0xFE LEN=9 SEQ=0 SYS=1 COMP=1 MSGID=0 PAYLOAD(9) CRC(2)
    QByteArray frame;
    frame.append(char(0xFE));
    frame.append(char(9)); // payload len
    frame.append(char(0)); // seq
    frame.append(char(1)); // sysid
    frame.append(char(1)); // compid
    frame.append(char(0)); // msgid (HEARTBEAT)
    frame.append(heartbeatPayload(4, 4));
    frame.append(char(0));
    frame.append(char(0));
    parser.feed(frame, [&](const DecodedFrame& f) { got = f; ++seen; });

    QCOMPARE(seen, 1);
    QVERIFY(!got.v2);
    QCOMPARE(got.sysid, 1);
    QCOMPARE(got.msgid, 0);
}

void TestMavlinkParser::protocolEmitsHeartbeat()
{
    MAVLinkProtocol p;
    QSignalSpy spy(&p, &MAVLinkProtocol::heartbeatReceived);

    p.feedBytesForTest(buildV2Frame(7, 1, msgid::Heartbeat,
                                    heartbeatPayload(4, 4)));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), 7);
    QCOMPARE(spy.first().at(1).toInt(), 1);
}

void TestMavlinkParser::protocolEmitsGlobalPositionInt()
{
    MAVLinkProtocol p;
    QSignalSpy spy(&p, &MAVLinkProtocol::globalPositionIntReceived);

    QByteArray payload(28, 0);
    auto put32 = [&](int offset, qint32 val) {
        payload[offset]     = char(val & 0xFF);
        payload[offset + 1] = char((val >>  8) & 0xFF);
        payload[offset + 2] = char((val >> 16) & 0xFF);
        payload[offset + 3] = char((val >> 24) & 0xFF);
    };
    auto put16 = [&](int offset, quint16 val) {
        payload[offset]     = char(val & 0xFF);
        payload[offset + 1] = char((val >> 8) & 0xFF);
    };
    put32(0, 1000);                  // time_boot_ms
    put32(4, qint32(21'0285110));    // lat *1e7 (~21.0285)
    put32(8, qint32(105'8048170));   // lon *1e7
    put32(12, qint32(123'456));      // alt_mm AMSL
    put32(16, qint32(50'000));       // relative_alt_mm = 50 m
    put16(26, quint16(12345));       // hdg cdeg = 123.45 deg

    p.feedBytesForTest(buildV2Frame(1, 1, msgid::GlobalPositionInt, payload));
    QCOMPARE(spy.count(), 1);
}

void TestMavlinkParser::protocolEmitsAttitude()
{
    MAVLinkProtocol p;
    QSignalSpy spy(&p, &MAVLinkProtocol::attitudeReceived);

    QByteArray payload(28, 0);
    auto putF = [&](int offset, float f) {
        std::memcpy(payload.data() + offset, &f, sizeof(float));
    };
    putF(4,  0.1f); // roll
    putF(8, -0.2f); // pitch
    putF(12, 1.0f); // yaw

    p.feedBytesForTest(buildV2Frame(1, 1, msgid::Attitude, payload));
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestMavlinkParser)
#include "tst_mavlink_parser.moc"
