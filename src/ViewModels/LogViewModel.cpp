#include "LogViewModel.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>

#include "Logging/EventLogger.h"
#include "Logging/MemoryLogSink.h"

namespace gcs::viewmodels {

using gcs::logging::EventLogger;
using gcs::logging::MemoryLogSink;

LogViewModel::LogViewModel(EventLogger* logger,
                           std::shared_ptr<MemoryLogSink> mem,
                           QObject* parent)
    : QObject(parent), m_logger(logger), m_mem(std::move(mem))
{
    if (m_mem) {
        connect(m_mem.get(), &MemoryLogSink::eventAppended,
                this, [this] { emit eventsChanged(); });
    }
}

QStringList LogViewModel::recentEvents() const
{
    return m_mem ? m_mem->formattedEvents() : QStringList{};
}

int LogViewModel::eventCount() const
{
    return m_mem ? m_mem->size() : 0;
}

void LogViewModel::setCurrentLogPath(const QString& path)
{
    if (m_logPath == path) return;
    m_logPath = path;
    emit currentLogPathChanged();
}

void LogViewModel::openLogFolder() const
{
    if (m_logPath.isEmpty()) return;
    const QFileInfo fi(m_logPath);
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
}

void LogViewModel::copyLogPath() const
{
    if (auto* cb = QGuiApplication::clipboard()) {
        cb->setText(m_logPath);
    }
}

} // namespace gcs::viewmodels
