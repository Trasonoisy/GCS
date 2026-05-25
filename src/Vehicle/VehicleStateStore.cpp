#include "VehicleStateStore.h"

#include <cmath>

namespace gcs::vehicle {

namespace {
bool nearlyEqual(double a, double b, double eps = 1e-9)
{
    return std::fabs(a - b) <= eps;
}
} // namespace

VehicleStateStore::VehicleStateStore(QObject* parent) : QObject(parent) {}

void VehicleStateStore::applyState(const VehicleState& next)
{
    m_state = next;
    emit stateChanged();
}

void VehicleStateStore::updateHeartbeat(int systemId,
                                        int componentId,
                                        const QString& vehicleType,
                                        const QString& autopilotType,
                                        qint64 utcMs)
{
    bool changed = false;
    if (m_state.systemId != systemId)           { m_state.systemId = systemId; changed = true; }
    if (m_state.componentId != componentId)     { m_state.componentId = componentId; changed = true; }
    if (m_state.vehicleType != vehicleType)     { m_state.vehicleType = vehicleType; changed = true; }
    if (m_state.autopilotType != autopilotType) { m_state.autopilotType = autopilotType; changed = true; }
    if (m_state.lastHeartbeatUtcMs != utcMs)    { m_state.lastHeartbeatUtcMs = utcMs; changed = true; }
    // A fresh heartbeat means the link is live.
    if (m_state.linkStatus != LinkStatus::Connected) {
        m_state.linkStatus = LinkStatus::Connected;
        changed = true;
    }
    if (changed) emit stateChanged();
}

void VehicleStateStore::updateFlightMode(const QString& mode)
{
    if (m_state.flightMode == mode) return;
    m_state.flightMode = mode;
    emit stateChanged();
}

void VehicleStateStore::updateArmed(bool armed)
{
    if (m_state.armed == armed) return;
    m_state.armed = armed;
    emit stateChanged();
}

void VehicleStateStore::updatePosition(double latDeg, double lonDeg, double relAltM)
{
    if (nearlyEqual(m_state.latitudeDeg, latDeg)
        && nearlyEqual(m_state.longitudeDeg, lonDeg)
        && nearlyEqual(m_state.relativeAltitudeM, relAltM)) {
        return;
    }
    m_state.latitudeDeg = latDeg;
    m_state.longitudeDeg = lonDeg;
    m_state.relativeAltitudeM = relAltM;
    emit stateChanged();
}

void VehicleStateStore::updateHeading(double headingDeg)
{
    if (nearlyEqual(m_state.headingDeg, headingDeg)) return;
    m_state.headingDeg = headingDeg;
    emit stateChanged();
}

void VehicleStateStore::updateAttitude(double rollDeg, double pitchDeg, double yawDeg)
{
    if (nearlyEqual(m_state.rollDeg, rollDeg)
        && nearlyEqual(m_state.pitchDeg, pitchDeg)
        && nearlyEqual(m_state.headingDeg, yawDeg)) {
        return;
    }
    m_state.rollDeg    = rollDeg;
    m_state.pitchDeg   = pitchDeg;
    m_state.headingDeg = yawDeg;
    emit stateChanged();
}

void VehicleStateStore::updateGroundSpeed(double mps)
{
    if (nearlyEqual(m_state.groundSpeedMps, mps)) return;
    m_state.groundSpeedMps = mps;
    emit stateChanged();
}

void VehicleStateStore::updateBattery(double voltage, double percent)
{
    if (nearlyEqual(m_state.batteryVoltage, voltage)
        && nearlyEqual(m_state.batteryPercent, percent)) {
        return;
    }
    m_state.batteryVoltage = voltage;
    m_state.batteryPercent = percent;
    emit stateChanged();
}

void VehicleStateStore::updateGps(int fixType, int satellitesVisible)
{
    if (m_state.gpsFixType == fixType
        && m_state.satellitesVisible == satellitesVisible) {
        return;
    }
    m_state.gpsFixType = fixType;
    m_state.satellitesVisible = satellitesVisible;
    emit stateChanged();
}

void VehicleStateStore::updateLinkStatus(LinkStatus status)
{
    if (m_state.linkStatus == status) return;
    m_state.linkStatus = status;
    emit stateChanged();
}

void VehicleStateStore::setLinkKind(gcs::comms::LinkKind kind)
{
    if (m_state.linkKind == kind) return;
    m_state.linkKind = kind;
    emit stateChanged();
}

void VehicleStateStore::markSimulated(bool simulated)
{
    if (m_state.simulated == simulated) return;
    m_state.simulated = simulated;
    emit stateChanged();
}

} // namespace gcs::vehicle
