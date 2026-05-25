#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include "Logging/FileLogSink.h"
#include "Logging/LogEvent.h"

using gcs::logging::FileLogSink;
using gcs::logging::LogEvent;

class TestFileLogSink : public QObject
{
    Q_OBJECT
private slots:
    void writesJsonLinePerEvent();
    void writeAfterCloseIsHarmless();
    void openFailureLeavesSinkUsableButNoOp();
    void defaultLogPathIsAJsonlFile();
};

void TestFileLogSink::writesJsonLinePerEvent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("session.jsonl");

    FileLogSink sink;
    QVERIFY(sink.open(path));
    QVERIFY(sink.isOpen());
    QCOMPARE(sink.filePath(), path);

    LogEvent e;
    e.timestampUtcMs = 1'700'000'000'000LL;
    e.category = "App"; e.severity = "info"; e.message = "started";
    e.metadata.insert("k", 42);
    sink.write(e);

    LogEvent e2;
    e2.timestampUtcMs = 1'700'000'000'500LL;
    e2.category = "Link"; e2.severity = "warn"; e2.message = "stale";
    sink.write(e2);
    sink.close();

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray all = f.readAll();
    f.close();
    const auto lines = all.split('\n');
    // 2 records + trailing empty line from '\n' on the last entry.
    QCOMPARE(lines.count(), 3);

    const auto d1 = QJsonDocument::fromJson(lines.at(0));
    QVERIFY(d1.isObject());
    QCOMPARE(d1.object().value("category").toString(), QStringLiteral("App"));
    QCOMPARE(d1.object().value("message").toString(),  QStringLiteral("started"));
    QCOMPARE(d1.object().value("meta").toObject().value("k").toInt(), 42);

    const auto d2 = QJsonDocument::fromJson(lines.at(1));
    QCOMPARE(d2.object().value("severity").toString(), QStringLiteral("warn"));
}

void TestFileLogSink::writeAfterCloseIsHarmless()
{
    QTemporaryDir dir; QVERIFY(dir.isValid());
    FileLogSink sink;
    QVERIFY(sink.open(dir.filePath("x.jsonl")));
    sink.close();
    QVERIFY(!sink.isOpen());
    LogEvent e; e.message = "after close";
    sink.write(e); // must not crash, just no-op
}

void TestFileLogSink::openFailureLeavesSinkUsableButNoOp()
{
    FileLogSink sink;
    // Path with an unlikely-to-be-creatable directory.
    QVERIFY(!sink.open(QStringLiteral("/no/such/path/should/exist/log.jsonl")));
    QVERIFY(!sink.isOpen());
    QVERIFY(!sink.lastError().isEmpty());
    LogEvent e; e.message = "noop";
    sink.write(e); // must not crash
}

void TestFileLogSink::defaultLogPathIsAJsonlFile()
{
    const QString p = FileLogSink::defaultLogPathForNow();
    QVERIFY(!p.isEmpty());
    QVERIFY(p.endsWith(QStringLiteral(".jsonl")));
}

QTEST_MAIN(TestFileLogSink)
#include "tst_file_log_sink.moc"
