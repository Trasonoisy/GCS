#pragma once

#include <QList>
#include <QString>

namespace gcs::firmware { class FirmwarePlugin; }

namespace gcs::mission {

struct MissionPlan;

struct ValidationIssue
{
    enum class Severity { Info, Warning, Error };

    Severity severity = Severity::Info;
    int      itemIndex = -1; // -1 = mission-wide issue
    QString  message;
};

class ValidationResult
{
public:
    void addError  (int itemIndex, QString msg);
    void addWarning(int itemIndex, QString msg);
    void addInfo   (int itemIndex, QString msg);

    bool hasErrors()   const;
    bool hasWarnings() const;
    bool isValid()     const { return !hasErrors(); }

    const QList<ValidationIssue>& issues() const { return m_issues; }
    QStringList errorMessages()   const;
    QStringList warningMessages() const;

private:
    QList<ValidationIssue> m_issues;
};

// Pure-functional validator. Holds no state of its own — pass the firmware
// plugin per call so the same validator works across vehicles.
class MissionValidator
{
public:
    struct Bounds {
        double minAltitudeM = -50.0;   // below home: small allowance
        double maxAltitudeM = 5000.0;  // FAA-style high ceiling
        // Warning threshold for distance between consecutive waypoints.
        double longLegWarnKm = 10.0;
    };

    MissionValidator() = default;
    explicit MissionValidator(const Bounds& bounds) : m_bounds(bounds) {}

    // `firmware` may be null — in that case command-support checks are skipped
    // with a Warning instead of Error (firmware plugins return an empty
    // command list in Phase 1/2).
    ValidationResult validate(const MissionPlan& plan,
                              const gcs::firmware::FirmwarePlugin* firmware) const;

    const Bounds& bounds() const { return m_bounds; }

private:
    Bounds m_bounds;
};

// Great-circle distance helper, exposed for tests.
double haversineKm(double lat1Deg, double lon1Deg,
                   double lat2Deg, double lon2Deg);

} // namespace gcs::mission
