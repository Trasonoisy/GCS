#pragma once

#include <QString>
#include <QStringList>

namespace gcs::safety {

// Plain-data decision returned by every SafetyGate check. The reason and
// warning fields are intended for human display in the UI.
//
// Decision precedence (UI must enforce):
//   - allowed = false  -> action is blocked, show `reason`
//   - requiresConfirmation -> show `confirmationText`, ask the operator
//   - otherwise the action may proceed; surface `warnings` if any
struct SafetyDecision
{
    bool        allowed              = false;
    QString     reason;
    QStringList warnings;
    bool        requiresConfirmation = false;
    QString     confirmationText;

    static SafetyDecision allow(QStringList warnings = {})
    {
        SafetyDecision d;
        d.allowed = true;
        d.warnings = std::move(warnings);
        return d;
    }

    static SafetyDecision block(QString reason)
    {
        SafetyDecision d;
        d.allowed = false;
        d.reason  = std::move(reason);
        return d;
    }

    static SafetyDecision needConfirm(QString reason, QString confirmText)
    {
        SafetyDecision d;
        d.allowed              = true;
        d.requiresConfirmation = true;
        d.reason               = std::move(reason);
        d.confirmationText     = std::move(confirmText);
        return d;
    }
};

} // namespace gcs::safety
