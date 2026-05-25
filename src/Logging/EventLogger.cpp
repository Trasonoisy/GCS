#include "EventLogger.h"

#include <QDateTime>

#include "LogSink.h"

namespace gcs::logging {

EventLogger::EventLogger(QObject* parent) : QObject(parent) {}
EventLogger::~EventLogger() = default;

void EventLogger::addSink(std::shared_ptr<LogSink> sink)
{
    if (sink) m_sinks.push_back(std::move(sink));
}

void EventLogger::clearSinks()
{
    m_sinks.clear();
}

void EventLogger::log(const QString& category, const QString& severity,
                      const QString& message, const QVariantMap& meta)
{
    LogEvent e;
    e.timestampUtcMs = QDateTime::currentMSecsSinceEpoch();
    e.category       = category;
    e.severity       = severity;
    e.message        = message;
    e.metadata       = meta;

    for (auto& s : m_sinks) {
        if (s) s->write(e);
    }
    emit eventLogged(e);
}

void EventLogger::info(const QString& c, const QString& m, const QVariantMap& meta)
{ log(c, QString::fromLatin1(severity::Info),  m, meta); }

void EventLogger::warn(const QString& c, const QString& m, const QVariantMap& meta)
{ log(c, QString::fromLatin1(severity::Warn),  m, meta); }

void EventLogger::error(const QString& c, const QString& m, const QVariantMap& meta)
{ log(c, QString::fromLatin1(severity::Error), m, meta); }

} // namespace gcs::logging
