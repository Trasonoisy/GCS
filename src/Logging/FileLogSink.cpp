#include "FileLogSink.h"

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace gcs::logging {

FileLogSink::FileLogSink() = default;
FileLogSink::~FileLogSink() { close(); }

bool FileLogSink::open(const QString& path)
{
    close();
    auto file = std::make_unique<QFile>(path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_lastError = file->errorString();
        return false;
    }
    m_file = std::move(file);
    m_path = path;
    m_lastError.clear();
    return true;
}

void FileLogSink::close()
{
    if (m_file) {
        m_file->close();
        m_file.reset();
    }
}

bool FileLogSink::isOpen() const
{
    return m_file && m_file->isOpen();
}

void FileLogSink::write(const LogEvent& e)
{
    if (!isOpen()) return;

    QJsonObject o;
    o.insert("ts",       e.timestampUtcMs);
    o.insert("category", e.category);
    o.insert("severity", e.severity);
    o.insert("message",  e.message);
    if (!e.metadata.isEmpty()) {
        o.insert("meta", QJsonObject::fromVariantMap(e.metadata));
    }
    const QByteArray line = QJsonDocument(o).toJson(QJsonDocument::Compact) + '\n';
    m_file->write(line);
    m_file->flush();
}

QString FileLogSink::defaultLogDirectory()
{
    QStringList roots;
    roots.append(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    roots.append(QDir::homePath() + QStringLiteral("/LabGCS"));
    roots.append(QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                 + QStringLiteral("/LabGCS"));
    roots.append(QDir::tempPath() + QStringLiteral("/LabGCS"));

    for (const QString& root : roots) {
        if (root.isEmpty()) continue;

        QDir dir(root + QStringLiteral("/logs"));
        if (dir.exists() || dir.mkpath(QStringLiteral("."))) {
            return dir.absolutePath();
        }
    }
    return {};
}

QString FileLogSink::defaultLogPathForNow()
{
    const QString dir = defaultLogDirectory();
    if (dir.isEmpty()) return {};
    const QString stamp = QDateTime::currentDateTimeUtc()
                          .toString(QStringLiteral("yyyyMMdd-HHmmss"));
    return QStringLiteral("%1/labgcs-%2.jsonl").arg(dir, stamp);
}

} // namespace gcs::logging
