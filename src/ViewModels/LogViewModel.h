#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

namespace gcs::logging {
class EventLogger;
class MemoryLogSink;
}

namespace gcs::viewmodels {

// QML-facing adapter for the EventLogger + MemoryLogSink. Exposes:
//   - current log file path
//   - recent formatted events (bound by QML if needed)
//   - openLogFolder() to surface the JSONL file in the OS file manager
class LogViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString     currentLogPath READ currentLogPath WRITE setCurrentLogPath NOTIFY currentLogPathChanged)
    Q_PROPERTY(QStringList recentEvents   READ recentEvents   NOTIFY eventsChanged)
    Q_PROPERTY(int         eventCount     READ eventCount     NOTIFY eventsChanged)

public:
    LogViewModel(gcs::logging::EventLogger* logger,
                 std::shared_ptr<gcs::logging::MemoryLogSink> mem,
                 QObject* parent = nullptr);

    QString     currentLogPath() const { return m_logPath; }
    QStringList recentEvents()   const;
    int         eventCount()     const;

    void setCurrentLogPath(const QString& path);

    Q_INVOKABLE void openLogFolder() const;
    Q_INVOKABLE void copyLogPath() const;

signals:
    void currentLogPathChanged();
    void eventsChanged();

private:
    gcs::logging::EventLogger*                   m_logger;
    std::shared_ptr<gcs::logging::MemoryLogSink> m_mem;
    QString                                      m_logPath;
};

} // namespace gcs::viewmodels
