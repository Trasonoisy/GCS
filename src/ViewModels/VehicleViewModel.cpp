#include "VehicleViewModel.h"

#include <QVariantMap>
#include <QtMath>
#include <cmath>

#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

namespace gcs::viewmodels {

using gcs::vehicle::LinkStatus;
using gcs::vehicle::MultiVehicleManager;
using gcs::vehicle::Vehicle;
using gcs::vehicle::VehicleState;

namespace {
const VehicleState kEmpty{}; // returned when no active vehicle

QString linkStatusToString(LinkStatus s)
{
    switch (s) {
        case LinkStatus::Disconnected: return QStringLiteral("Disconnected");
        case LinkStatus::Connected:    return QStringLiteral("Connected");
        case LinkStatus::Stale:        return QStringLiteral("Stale");
    }
    return QStringLiteral("Unknown");
}
} // namespace

VehicleViewModel::VehicleViewModel(MultiVehicleManager* manager, QObject* parent)
    : QObject(parent), m_manager(manager)
{
    Q_ASSERT(m_manager);
    connect(m_manager, &MultiVehicleManager::activeVehicleChanged,
            this, &VehicleViewModel::onActiveVehicleChanged);
    rebind(m_manager->activeVehicle());

    // Drive the heartbeat-age label on a 1 Hz tick so the UI shows it
    // climbing without us re-emitting on every telemetry sample.
    m_heartbeatAgeTimer.setInterval(1000);
    connect(&m_heartbeatAgeTimer, &QTimer::timeout,
            this, &VehicleViewModel::heartbeatAgeChanged);
    m_heartbeatAgeTimer.start();
}

void VehicleViewModel::onActiveVehicleChanged(Vehicle* vehicle)
{
    rebind(vehicle);
}

void VehicleViewModel::rebind(Vehicle* vehicle)
{
    if (m_vehicle) {
        disconnect(m_vehicle->stateStore(), nullptr, this, nullptr);
        disconnect(m_vehicle, nullptr, this, nullptr);
    }

    m_vehicle = vehicle;
    m_events.clear();
    m_trail.clear();
    m_haveLastTrailPoint = false;
    m_haveLastBearingPoint = false;
    m_displayHeadingDeg = 0.0;

    if (m_vehicle) {
        // Drive both the generic UI refresh AND the trail builder from the
        // same store-changed signal so the map polyline stays in lock-step
        // with the rest of the telemetry display.
        connect(m_vehicle->stateStore(),
                &gcs::vehicle::VehicleStateStore::stateChanged,
                this, &VehicleViewModel::onVehicleStateChanged);
        connect(m_vehicle, &Vehicle::eventAppended,
                this, &VehicleViewModel::onVehicleEventAppended);

        // Backfill any events the vehicle already collected.
        for (const auto& e : m_vehicle->events()) {
            m_events.append(e);
        }
    }

    emit changed();
    emit eventLogChanged();
    emit trailChanged();
}

void VehicleViewModel::onVehicleStateChanged()
{
    if (m_vehicle) {
        const auto& s = m_vehicle->stateStore()->state();
        appendTrailPointIfMoved(s.latitudeDeg, s.longitudeDeg);
    }
    emit changed();
}

void VehicleViewModel::appendTrailPointIfMoved(double lat, double lon)
{
    // Skip points before we have any GPS fix (avoids a (0,0) ghost segment
    // from the equator to the first valid sample).
    if (!std::isfinite(lat) || !std::isfinite(lon)) return;
    if (lat == 0.0 && lon == 0.0) return;

    // ---- bearing (marker heading) -----------------------------------------
    //
    // Run BEFORE the trail-threshold gate so the marker still tracks
    // direction even when individual telemetry samples are tiny.
    //
    // We anchor against m_lastBearingLat/Lon (NOT the last trail point)
    // because the bearing threshold (~28 m) is much larger than the trail
    // threshold (~1.1 m). Anchoring on the trail point would make the
    // marker spin on every tight loiter circle — exactly the bug we are
    // fixing — since each ~1 m trail sample produces a wildly different
    // bearing as the vehicle orbits.
    if (m_haveLastBearingPoint) {
        const double bdLat = std::abs(lat - m_lastBearingLat);
        const double bdLon = std::abs(lon - m_lastBearingLon);
        if (bdLat >= kMinBearingDeltaDeg || bdLon >= kMinBearingDeltaDeg) {
            // Initial-bearing formula on a sphere (Aviation Formulary §1).
            const double phi1    = qDegreesToRadians(m_lastBearingLat);
            const double phi2    = qDegreesToRadians(lat);
            const double dLambda = qDegreesToRadians(lon - m_lastBearingLon);
            const double y       = std::sin(dLambda) * std::cos(phi2);
            const double x       = std::cos(phi1) * std::sin(phi2)
                                 - std::sin(phi1) * std::cos(phi2) * std::cos(dLambda);
            double brg = qRadiansToDegrees(std::atan2(y, x));
            if (brg < 0.0) brg += 360.0;
            m_displayHeadingDeg = brg;
            m_lastBearingLat = lat;
            m_lastBearingLon = lon;
        }
    } else {
        // First valid sample: seed the bearing anchor; leave the marker
        // pointing at its default (north / 0°) until a real move happens.
        m_lastBearingLat = lat;
        m_lastBearingLon = lon;
        m_haveLastBearingPoint = true;
    }

    // ---- trail (fine-grained breadcrumb polyline) -------------------------
    if (m_haveLastTrailPoint) {
        const double dLat = std::abs(lat - m_lastTrailLat);
        const double dLon = std::abs(lon - m_lastTrailLon);
        if (dLat < kMinTrailDeltaDeg && dLon < kMinTrailDeltaDeg) return;
    }

    QVariantMap p;
    p[QStringLiteral("lat")] = lat;
    p[QStringLiteral("lon")] = lon;
    m_trail.append(p);
    while (m_trail.size() > kMaxTrailPoints) {
        m_trail.removeFirst();
    }
    m_lastTrailLat = lat;
    m_lastTrailLon = lon;
    m_haveLastTrailPoint = true;
    emit trailChanged();
}

bool VehicleViewModel::hasValidPosition() const
{
    if (!m_vehicle) return false;
    const auto& s = m_vehicle->stateStore()->state();
    if (!std::isfinite(s.latitudeDeg) || !std::isfinite(s.longitudeDeg)) return false;
    if (s.latitudeDeg == 0.0 && s.longitudeDeg == 0.0) return false;
    return true;
}

void VehicleViewModel::clearTrail()
{
    if (m_trail.isEmpty() && !m_haveLastTrailPoint) return;
    m_trail.clear();
    m_haveLastTrailPoint = false;
    // Leave m_displayHeadingDeg at its current value — clearing the trail
    // is a visual operation; we don't want the marker to suddenly snap to
    // north just because the operator wiped the breadcrumbs.
    emit trailChanged();
}

void VehicleViewModel::onVehicleEventAppended(const QString& message)
{
    m_events.append(message);
    while (m_events.size() > kMaxUiEvents) {
        m_events.removeFirst();
    }
    emit eventLogChanged();
}

bool VehicleViewModel::simulated() const
{
    return m_vehicle ? m_vehicle->stateStore()->state().simulated : false;
}

QString VehicleViewModel::vehicleLabel() const
{
    if (!m_vehicle) return QStringLiteral("(none)");
    const auto& s = m_vehicle->stateStore()->state();
    return QStringLiteral("%1 #%2")
        .arg(s.vehicleType.isEmpty() ? QStringLiteral("vehicle") : s.vehicleType)
        .arg(s.systemId);
}

int     VehicleViewModel::systemId()          const { return m_vehicle ? m_vehicle->stateStore()->state().systemId          : kEmpty.systemId; }
int     VehicleViewModel::componentId()       const { return m_vehicle ? m_vehicle->stateStore()->state().componentId       : kEmpty.componentId; }
QString VehicleViewModel::vehicleType()       const { return m_vehicle ? m_vehicle->stateStore()->state().vehicleType       : kEmpty.vehicleType; }
QString VehicleViewModel::autopilotType()     const { return m_vehicle ? m_vehicle->stateStore()->state().autopilotType     : kEmpty.autopilotType; }
QString VehicleViewModel::flightMode()        const { return m_vehicle ? m_vehicle->stateStore()->state().flightMode        : kEmpty.flightMode; }
bool    VehicleViewModel::armed()             const { return m_vehicle ? m_vehicle->stateStore()->state().armed             : kEmpty.armed; }
double  VehicleViewModel::latitudeDeg()       const { return m_vehicle ? m_vehicle->stateStore()->state().latitudeDeg       : kEmpty.latitudeDeg; }
double  VehicleViewModel::longitudeDeg()      const { return m_vehicle ? m_vehicle->stateStore()->state().longitudeDeg      : kEmpty.longitudeDeg; }
double  VehicleViewModel::relativeAltitudeM() const { return m_vehicle ? m_vehicle->stateStore()->state().relativeAltitudeM : kEmpty.relativeAltitudeM; }
double  VehicleViewModel::headingDeg()        const { return m_vehicle ? m_vehicle->stateStore()->state().headingDeg        : kEmpty.headingDeg; }
double  VehicleViewModel::rollDeg()           const { return m_vehicle ? m_vehicle->stateStore()->state().rollDeg           : kEmpty.rollDeg; }
double  VehicleViewModel::pitchDeg()          const { return m_vehicle ? m_vehicle->stateStore()->state().pitchDeg          : kEmpty.pitchDeg; }
double  VehicleViewModel::groundSpeedMps()    const { return m_vehicle ? m_vehicle->stateStore()->state().groundSpeedMps    : kEmpty.groundSpeedMps; }
double  VehicleViewModel::batteryVoltage()    const { return m_vehicle ? m_vehicle->stateStore()->state().batteryVoltage    : kEmpty.batteryVoltage; }
double  VehicleViewModel::batteryPercent()    const { return m_vehicle ? m_vehicle->stateStore()->state().batteryPercent    : kEmpty.batteryPercent; }
int     VehicleViewModel::gpsFixType()        const { return m_vehicle ? m_vehicle->stateStore()->state().gpsFixType        : kEmpty.gpsFixType; }
int     VehicleViewModel::satellitesVisible() const { return m_vehicle ? m_vehicle->stateStore()->state().satellitesVisible : kEmpty.satellitesVisible; }

QString VehicleViewModel::linkStatusText() const
{
    return linkStatusToString(m_vehicle ? m_vehicle->stateStore()->state().linkStatus
                                        : kEmpty.linkStatus);
}

qint64 VehicleViewModel::heartbeatAgeMs() const
{
    if (!m_vehicle) return -1;
    const auto& s = m_vehicle->stateStore()->state();
    if (s.lastHeartbeatUtcMs <= 0) return -1;
    return QDateTime::currentMSecsSinceEpoch() - s.lastHeartbeatUtcMs;
}

QStringList VehicleViewModel::eventLog() const { return m_events; }

} // namespace gcs::viewmodels
