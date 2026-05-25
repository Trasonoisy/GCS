#include <QtTest/QtTest>
#include <memory>

#include "Logging/EventLogger.h"
#include "Logging/LogEvent.h"
#include "Logging/MemoryLogSink.h"
#include "Logging/OperatorActionLogger.h"

using gcs::logging::EventLogger;
using gcs::logging::MemoryLogSink;
using gcs::logging::OperatorActionLogger;

class TestOperatorActionLogger : public QObject
{
    Q_OBJECT
private slots:
    void recordsUnderOperatorActionCategory();
    void detailIsAppendedToMessage();
    void actionEchoedIntoMetadata();
    void nullBaseIsHarmless();
};

void TestOperatorActionLogger::recordsUnderOperatorActionCategory()
{
    EventLogger lg;
    auto m = std::make_shared<MemoryLogSink>();
    lg.addSink(m);
    OperatorActionLogger ops(&lg);
    ops.recordAction(QStringLiteral("Enable manual control"));
    QCOMPARE(m->size(), 1);
    QCOMPARE(m->events().first().category, QStringLiteral("OperatorAction"));
    QCOMPARE(m->events().first().severity, QStringLiteral("info"));
}

void TestOperatorActionLogger::detailIsAppendedToMessage()
{
    EventLogger lg;
    auto m = std::make_shared<MemoryLogSink>();
    lg.addSink(m);
    OperatorActionLogger ops(&lg);
    ops.recordAction(QStringLiteral("Connect"), QStringLiteral("UDP 14550"));
    QVERIFY(m->events().first().message.contains("Connect"));
    QVERIFY(m->events().first().message.contains("UDP 14550"));
}

void TestOperatorActionLogger::actionEchoedIntoMetadata()
{
    EventLogger lg;
    auto m = std::make_shared<MemoryLogSink>();
    lg.addSink(m);
    OperatorActionLogger ops(&lg);
    ops.recordAction(QStringLiteral("Save mission"),
                     QStringLiteral("file.plan"),
                     {{QStringLiteral("items"), 3}});
    const auto& meta = m->events().first().metadata;
    QCOMPARE(meta.value("action").toString(), QStringLiteral("Save mission"));
    QCOMPARE(meta.value("items").toInt(), 3);
}

void TestOperatorActionLogger::nullBaseIsHarmless()
{
    OperatorActionLogger ops(nullptr);
    ops.recordAction(QStringLiteral("noop")); // must not crash
}

QTEST_MAIN(TestOperatorActionLogger)
#include "tst_operator_action_logger.moc"
