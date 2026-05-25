#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>
#include <vector>

#include "LogEvent.h"

namespace gcs::logging {

class LogSink;

// Central log fan-out. The application creates one EventLogger, attaches
// sinks, and connects signals from across the codebase to its `log*`
// methods. Downstream classes do NOT need to know about the logger —
// `main.cpp` wires them together with Qt signal/slot connections.
//
// SAFETY: This is a pure observer. It cannot influence the control path.
class EventLogger : public QObject
{
    Q_OBJECT
public:
    explicit EventLogger(QObject* parent = nullptr);
    ~EventLogger() override;

    // Sinks are kept by shared_ptr so the same MemoryLogSink can also be
    // held by a ViewModel for QML binding.
    void addSink(std::shared_ptr<LogSink> sink);
    void clearSinks();
    int  sinkCount() const { return int(m_sinks.size()); }

    // Low-level entry point.
    void log(const QString& category, const QString& severity,
             const QString& message, const QVariantMap& meta = {});

    // Convenience.
    void info (const QString& category, const QString& message, const QVariantMap& meta = {});
    void warn (const QString& category, const QString& message, const QVariantMap& meta = {});
    void error(const QString& category, const QString& message, const QVariantMap& meta = {});

signals:
    // Emitted after every event has been fanned out to all sinks. Useful
    // for the active Vehicle's event-log mirror in `main.cpp`.
    void eventLogged(const gcs::logging::LogEvent& e);

private:
    std::vector<std::shared_ptr<LogSink>> m_sinks;
};

} // namespace gcs::logging
