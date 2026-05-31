#include <QtTest/QtTest>

#include "Comms/LinkInterface.h"
#include "Comms/LinkKind.h"
#include "Manual/MavlinkManualControlSink.h"
#include "Protocol/MavlinkFrame.h"

using gcs::comms::LinkInterface;
using gcs::comms::LinkKind;
using gcs::manual::MavlinkManualControlSink;
using namespace gcs::protocol;

class CaptureLink : public LinkInterface
{
    Q_OBJECT
public:
    explicit CaptureLink(LinkKind kind = LinkKind::Udp, QObject* parent = nullptr)
        : LinkInterface(parent), m_kind(kind)
    {
    }

    QString name() const override { return QStringLiteral("capture"); }
    bool isConnected() const override { return m_connected; }
    LinkKind kind() const override { return m_kind; }

    QByteArray lastWrittenBytes() const { return m_lastWritten; }
    int writeCount() const { return m_writeCount; }

public slots:
    void connectLink() override
    {
        m_connected = true;
        emit connectedChanged(true);
    }

    void disconnectLink() override
    {
        m_connected = false;
        emit connectedChanged(false);
    }

    void writeBytes(const QByteArray& bytes) override
    {
        if (!m_connected) {
            emit errorOccurred(QStringLiteral("not connected"));
            return;
        }
        m_lastWritten = bytes;
        ++m_writeCount;
    }

private:
    LinkKind m_kind;
    bool m_connected = false;
    QByteArray m_lastWritten;
    int m_writeCount = 0;
};

class TestMavlinkManualControlSink : public QObject
{
    Q_OBJECT
private slots:
    void payloadUsesCommonWireLayout();
    void sinkWritesManualControlFrame();
    void sinkClampsAxesBeforeWriting();
    void sinkRefusesNonNetworkLink();
};

void TestMavlinkManualControlSink::payloadUsesCommonWireLayout()
{
    msg::ManualControl in;
    in.target = 1;
    in.x = -1000;
    in.y = 250;
    in.z = 600;
    in.r = -125;
    in.buttons = 0x0005;

    const QByteArray payload = msg::encodeManualControl(in);
    QCOMPARE(payload.size(), 11);

    msg::ManualControl out;
    QVERIFY(msg::decodeManualControl(payload, out));
    QCOMPARE(out.x, in.x);
    QCOMPARE(out.y, in.y);
    QCOMPARE(out.z, in.z);
    QCOMPARE(out.r, in.r);
    QCOMPARE(out.buttons, in.buttons);
    QCOMPARE(out.target, in.target);
}

void TestMavlinkManualControlSink::sinkWritesManualControlFrame()
{
    CaptureLink link(LinkKind::Udp);
    link.connectLink();

    MavlinkManualControlSink sink(/*targetSystemId*/1, &link);
    sink.onManualControlSample(100, -200, 500, 900, 3);

    QCOMPARE(link.writeCount(), 1);

    MavlinkV2Parser parser;
    DecodedFrame decoded;
    bool got = false;
    parser.feed(link.lastWrittenBytes(), [&](const DecodedFrame& f) {
        decoded = f;
        got = true;
    });

    QVERIFY(got);
    QCOMPARE(decoded.sysid, 255);
    QCOMPARE(decoded.compid, 190);
    QCOMPARE(decoded.msgid, msgid::ManualControl);

    msg::ManualControl out;
    QVERIFY(msg::decodeManualControl(decoded.payload, out));
    QCOMPARE(out.target, uint8_t(1));
    QCOMPARE(out.x, int16_t(100));
    QCOMPARE(out.y, int16_t(-200));
    QCOMPARE(out.z, int16_t(500));
    QCOMPARE(out.r, int16_t(900));
    QCOMPARE(out.buttons, uint16_t(3));
}

void TestMavlinkManualControlSink::sinkClampsAxesBeforeWriting()
{
    CaptureLink link(LinkKind::Udp);
    link.connectLink();

    MavlinkManualControlSink sink(/*targetSystemId*/1, &link);
    sink.onManualControlSample(2000, -2000, -1, 2001, 0);

    MavlinkV2Parser parser;
    DecodedFrame decoded;
    bool got = false;
    parser.feed(link.lastWrittenBytes(), [&](const DecodedFrame& f) {
        decoded = f;
        got = true;
    });
    QVERIFY(got);

    msg::ManualControl out;
    QVERIFY(msg::decodeManualControl(decoded.payload, out));
    QCOMPARE(out.x, int16_t(1000));
    QCOMPARE(out.y, int16_t(-1000));
    QCOMPARE(out.z, int16_t(0));
    QCOMPARE(out.r, int16_t(1000));
}

void TestMavlinkManualControlSink::sinkRefusesNonNetworkLink()
{
    CaptureLink link(LinkKind::Serial);
    link.connectLink();

    MavlinkManualControlSink sink(/*targetSystemId*/1, &link);
    sink.onManualControlSample(100, 100, 500, 0, 0);

    QCOMPARE(link.writeCount(), 0);
}

QTEST_MAIN(TestMavlinkManualControlSink)
#include "tst_mavlink_manual_control_sink.moc"
