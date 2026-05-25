#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Logging/LogEvent.h"
#include "Logging/MemoryLogSink.h"

using gcs::logging::LogEvent;
using gcs::logging::MemoryLogSink;

class TestMemoryLogSink : public QObject
{
    Q_OBJECT
private slots:
    void writeAppendsAndEmits();
    void capacityEvictsOldest();
    void formatEventIsHumanReadable();
};

void TestMemoryLogSink::writeAppendsAndEmits()
{
    MemoryLogSink sink(/*capacity*/ 10);
    QSignalSpy spy(&sink, &MemoryLogSink::eventAppended);

    LogEvent e;
    e.timestampUtcMs = 1'700'000'000'000LL;
    e.category = "App"; e.severity = "info"; e.message = "started";
    sink.write(e);

    QCOMPARE(sink.size(), 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(sink.events().first().message, QStringLiteral("started"));
}

void TestMemoryLogSink::capacityEvictsOldest()
{
    MemoryLogSink sink(/*capacity*/ 3);
    for (int i = 0; i < 5; ++i) {
        LogEvent e; e.message = QStringLiteral("ev%1").arg(i);
        sink.write(e);
    }
    QCOMPARE(sink.size(), 3);
    QCOMPARE(sink.events().first().message,  QStringLiteral("ev2"));
    QCOMPARE(sink.events().last().message,   QStringLiteral("ev4"));
}

void TestMemoryLogSink::formatEventIsHumanReadable()
{
    LogEvent e;
    e.timestampUtcMs = 1'700'000'000'000LL;
    e.category = "Vehicle"; e.severity = "warn"; e.message = "Heartbeat stale";
    const QString line = MemoryLogSink::formatEvent(e);
    QVERIFY(line.contains("WARN"));
    QVERIFY(line.contains("Vehicle"));
    QVERIFY(line.contains("Heartbeat stale"));
}

QTEST_MAIN(TestMemoryLogSink)
#include "tst_memory_log_sink.moc"
