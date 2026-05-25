#include "OperatorActionLogger.h"

#include "EventLogger.h"
#include "LogEvent.h"

namespace gcs::logging {

void OperatorActionLogger::recordAction(const QString& action,
                                        const QString& detail,
                                        const QVariantMap& meta) const
{
    if (!m_base) return;
    QString message = action;
    if (!detail.isEmpty()) {
        message += QStringLiteral(" — ") + detail;
    }
    QVariantMap m = meta;
    m.insert(QStringLiteral("action"), action);
    m_base->log(QString::fromLatin1(category::OperatorAction),
                QString::fromLatin1(severity::Info),
                message, m);
}

} // namespace gcs::logging
