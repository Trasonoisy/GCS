#pragma once

#include <QString>
#include <QVariantMap>

namespace gcs::logging {

class EventLogger;

// Thin façade that records explicit operator actions under the
// `OperatorAction` category. Use this whenever a UI button click or
// equivalent operator decision needs to be auditable.
//
// SAFETY: We log the *intent* (e.g. "Operator enabled manual control") and
// the resulting decision separately when needed. SafetyGate rejections are
// logged under the `Safety` category by the relevant manager.
class OperatorActionLogger
{
public:
    explicit OperatorActionLogger(EventLogger* base) : m_base(base) {}

    // Records an operator action. `detail` is an optional human-readable
    // sentence; `meta` carries structured context (e.g. vehicle sysid).
    void recordAction(const QString& action,
                      const QString& detail = QString(),
                      const QVariantMap& meta = {}) const;

    EventLogger* base() const { return m_base; }

private:
    EventLogger* m_base;
};

} // namespace gcs::logging
