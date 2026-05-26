#include "MemoryLogSink.h"

#include <QDateTime>
#include <QTimeZone>

namespace gcs::logging {

MemoryLogSink::MemoryLogSink(int capacity, QObject* parent)
    : QObject(parent), m_capacity(qMax(1, capacity))
{
}

void MemoryLogSink::write(const LogEvent& e)
{
    m_buf.append(e);
    while (m_buf.size() > m_capacity) {
        m_buf.removeFirst();
    }
    emit eventAppended(e);
}

QStringList MemoryLogSink::formattedEvents() const
{
    QStringList out;
    out.reserve(m_buf.size());
    for (const auto& e : m_buf) {
        out.append(formatEvent(e));
    }
    return out;
}

QString MemoryLogSink::formatEvent(const LogEvent& e)
{
    const QString stamp = QDateTime::fromMSecsSinceEpoch(e.timestampUtcMs, QTimeZone::UTC)
                          .toString(QStringLiteral("HH:mm:ss"));
    return QStringLiteral("[%1 %2/%3] %4")
        .arg(stamp, e.severity.toUpper(), e.category, e.message);
}

} // namespace gcs::logging
