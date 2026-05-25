#include <QSignalSpy>
#include <QtTest/QtTest>
#include <memory>

#include "Logging/EventLogger.h"
#include "Logging/LogEvent.h"
#include "Logging/MemoryLogSink.h"

using gcs::logging::EventLogger;
using gcs::logging::LogEvent;
using gcs::logging::MemoryLogSink;

class TestEventLogger : public QObject
{
    Q_OBJECT
private slots:
    void fanOutsToAllSinks();
    void infoWarnErrorSetSeverityStrings();
    void eventLoggedSignalCarriesTheEvent();
    void clearSinksDropsConnections();
    void timestampPopulatedAutomatically();
};

void TestEventLogger::fanOutsToAllSinks()
{
    EventLogger lg;
    auto a = std::make_shared<MemoryLogSink>();
    auto b = std::make_shared<MemoryLogSink>();
    lg.addSink(a);
    lg.addSink(b);
    lg.info("App", "hello");
    QCOMPARE(a->size(), 1);
    QCOMPARE(b->size(), 1);
    QCOMPARE(a->events().first().message, QStringLiteral("hello"));
}

void TestEventLogger::infoWarnErrorSetSeverityStrings()
{
    EventLogger lg;
    auto m = std::make_shared<MemoryLogSink>();
    lg.addSink(m);
    lg.info ("c", "i");
    lg.warn ("c", "w");
    lg.error("c", "e");
    QCOMPARE(m->size(), 3);
    QCOMPARE(m->events().at(0).severity, QStringLiteral("info"));
    QCOMPARE(m->events().at(1).severity, QStringLiteral("warn"));
    QCOMPARE(m->events().at(2).severity, QStringLiteral("error"));
}

void TestEventLogger::eventLoggedSignalCarriesTheEvent()
{
    EventLogger lg;
    QSignalSpy spy(&lg, &EventLogger::eventLogged);
    lg.warn("Mission", "validation failed", {{QStringLiteral("errors"), 2}});
    QCOMPARE(spy.count(), 1);
    const auto e = spy.first().at(0).value<LogEvent>();
    QCOMPARE(e.category, QStringLiteral("Mission"));
    QCOMPARE(e.severity, QStringLiteral("warn"));
    QCOMPARE(e.metadata.value("errors").toInt(), 2);
}

void TestEventLogger::clearSinksDropsConnections()
{
    EventLogger lg;
    auto m = std::make_shared<MemoryLogSink>();
    lg.addSink(m);
    lg.info("c", "before clear");
    lg.clearSinks();
    lg.info("c", "after clear");
    QCOMPARE(m->size(), 1);
}

void TestEventLogger::timestampPopulatedAutomatically()
{
    EventLogger lg;
    auto m = std::make_shared<MemoryLogSink>();
    lg.addSink(m);
    const qint64 before = QDateTime::currentMSecsSinceEpoch();
    lg.info("c", "now");
    const qint64 after  = QDateTime::currentMSecsSinceEpoch();
    const qint64 ts = m->events().first().timestampUtcMs;
    QVERIFY(ts >= before && ts <= after);
}

QTEST_MAIN(TestEventLogger)
#include "tst_event_logger.moc"
