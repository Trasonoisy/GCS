#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include "LogSink.h"

namespace gcs::logging {

// Keeps the most recent `capacity` events in memory. Used to feed a UI panel
// and to assert behavior in tests. Emits `eventAppended` whenever a new
// event arrives so QML / consumers can react incrementally.
class MemoryLogSink : public QObject, public LogSink
{
    Q_OBJECT
public:
    explicit MemoryLogSink(int capacity = 500, QObject* parent = nullptr);

    void write(const LogEvent& e) override;

    QList<LogEvent> events() const { return m_buf; }
    int             size()   const { return m_buf.size(); }
    int             capacity() const { return m_capacity; }

    // Human-readable formatted lines for binding into QML ListView.
    QStringList formattedEvents() const;

    static QString formatEvent(const LogEvent& e);

signals:
    void eventAppended(const gcs::logging::LogEvent& e);

private:
    int             m_capacity;
    QList<LogEvent> m_buf;
};

} // namespace gcs::logging
