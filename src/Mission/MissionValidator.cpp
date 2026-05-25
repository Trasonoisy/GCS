#include "MissionValidator.h"

#include <QtMath>
#include <cmath>

#include "Firmware/FirmwarePlugin.h"
#include "MissionItem.h"
#include "MissionPlan.h"

namespace gcs::mission {

void ValidationResult::addError(int itemIndex, QString msg)
{
    m_issues.append({ValidationIssue::Severity::Error, itemIndex, std::move(msg)});
}
void ValidationResult::addWarning(int itemIndex, QString msg)
{
    m_issues.append({ValidationIssue::Severity::Warning, itemIndex, std::move(msg)});
}
void ValidationResult::addInfo(int itemIndex, QString msg)
{
    m_issues.append({ValidationIssue::Severity::Info, itemIndex, std::move(msg)});
}

bool ValidationResult::hasErrors() const
{
    for (const auto& i : m_issues) {
        if (i.severity == ValidationIssue::Severity::Error) return true;
    }
    return false;
}

bool ValidationResult::hasWarnings() const
{
    for (const auto& i : m_issues) {
        if (i.severity == ValidationIssue::Severity::Warning) return true;
    }
    return false;
}

QStringList ValidationResult::errorMessages() const
{
    QStringList out;
    for (const auto& i : m_issues) {
        if (i.severity == ValidationIssue::Severity::Error) out << i.message;
    }
    return out;
}

QStringList ValidationResult::warningMessages() const
{
    QStringList out;
    for (const auto& i : m_issues) {
        if (i.severity == ValidationIssue::Severity::Warning) out << i.message;
    }
    return out;
}

double haversineKm(double lat1Deg, double lon1Deg,
                   double lat2Deg, double lon2Deg)
{
    constexpr double kEarthKm = 6371.0088;
    const double lat1 = qDegreesToRadians(lat1Deg);
    const double lat2 = qDegreesToRadians(lat2Deg);
    const double dLat = lat2 - lat1;
    const double dLon = qDegreesToRadians(lon2Deg - lon1Deg);
    const double a = std::sin(dLat / 2) * std::sin(dLat / 2)
                   + std::cos(lat1) * std::cos(lat2)
                   * std::sin(dLon / 2) * std::sin(dLon / 2);
    const double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return kEarthKm * c;
}

namespace {

bool itemNeedsCoordinates(int command)
{
    // RTL has no coordinates; all our other commands do.
    return command != cmd::NavReturnToLaunch;
}

bool frameAccepted(int frame, const gcs::firmware::FirmwarePlugin* fw)
{
    // For now we accept the relative-altitude frames our model targets. The
    // firmware-frame policy is consulted but only for telemetry — we don't
    // override its choice. Plenty of room to tighten this in Phase 3.
    Q_UNUSED(fw);
    return frame == gcs::mission::frame::GlobalRelativeAlt
        || frame == gcs::mission::frame::GlobalRelativeAltInt
        || frame == gcs::mission::frame::Global
        || frame == gcs::mission::frame::GlobalInt;
}

} // namespace

ValidationResult MissionValidator::validate(
    const MissionPlan& plan,
    const gcs::firmware::FirmwarePlugin* firmware) const
{
    ValidationResult r;

    if (plan.items.isEmpty()) {
        r.addError(-1, QStringLiteral("Mission is empty — add at least one waypoint."));
        return r;
    }

    QList<int> supported = firmware ? firmware->supportedMissionCommands() : QList<int>{};
    const bool firmwareCommandListEmpty = supported.isEmpty();

    for (int i = 0; i < plan.items.size(); ++i) {
        const MissionItem& item = plan.items.at(i);

        if (itemNeedsCoordinates(item.command)) {
            if (item.latitudeDeg < -90.0 || item.latitudeDeg > 90.0) {
                r.addError(i, QStringLiteral(
                    "Latitude %1° out of range [-90, 90]").arg(item.latitudeDeg, 0, 'f', 6));
            }
            if (item.longitudeDeg < -180.0 || item.longitudeDeg > 180.0) {
                r.addError(i, QStringLiteral(
                    "Longitude %1° out of range [-180, 180]").arg(item.longitudeDeg, 0, 'f', 6));
            }
            if (std::isnan(item.latitudeDeg) || std::isnan(item.longitudeDeg)) {
                r.addError(i, QStringLiteral("Latitude/longitude must be set"));
            }
        }

        if (item.altitudeM < m_bounds.minAltitudeM
            || item.altitudeM > m_bounds.maxAltitudeM) {
            r.addError(i, QStringLiteral(
                "Altitude %1 m outside safe bounds [%2, %3]")
                .arg(item.altitudeM, 0, 'f', 1)
                .arg(m_bounds.minAltitudeM, 0, 'f', 1)
                .arg(m_bounds.maxAltitudeM, 0, 'f', 1));
        }

        if (!frameAccepted(item.frame, firmware)) {
            r.addError(i, QStringLiteral("Unsupported frame: %1").arg(frameName(item.frame)));
        }

        if (firmwareCommandListEmpty) {
            // Phase 1/2: firmware plugins return an empty supported list (fail
            // closed for *real* upload). For mock-only validation we warn so
            // the user can still proceed against MockVehicle.
            r.addWarning(i, QStringLiteral(
                "Firmware command list is empty; %1 not verified against firmware.")
                .arg(commandName(item.command)));
        } else if (!supported.contains(item.command)) {
            r.addError(i, QStringLiteral("Command %1 not supported by firmware")
                .arg(commandName(item.command)));
        }

        if (item.seq != i) {
            r.addWarning(i, QStringLiteral("seq=%1 does not match position %2")
                .arg(item.seq).arg(i));
        }
    }

    for (int i = 1; i < plan.items.size(); ++i) {
        const auto& a = plan.items.at(i - 1);
        const auto& b = plan.items.at(i);
        if (!itemNeedsCoordinates(a.command) || !itemNeedsCoordinates(b.command)) {
            continue;
        }
        const double km = haversineKm(a.latitudeDeg, a.longitudeDeg,
                                      b.latitudeDeg, b.longitudeDeg);
        if (km > m_bounds.longLegWarnKm) {
            r.addWarning(i, QStringLiteral(
                "Leg %1 → %2 is %3 km (> %4 km warn threshold)")
                .arg(i - 1).arg(i)
                .arg(km, 0, 'f', 1)
                .arg(m_bounds.longLegWarnKm, 0, 'f', 1));
        }
    }

    return r;
}

} // namespace gcs::mission
