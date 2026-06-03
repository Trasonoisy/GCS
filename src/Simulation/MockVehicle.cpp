#include "MockVehicle.h"

#include <QDateTime>
#include <QtMath>

#include "MockMissionLink.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

namespace gcs::simulation {

using gcs::vehicle::LinkStatus;

namespace {
double clamp(double v, double lo, double hi)
{
    return qMax(lo, qMin(hi, v));
}

double normalizeHeading(double deg)
{
    double out = std::fmod(deg, 360.0);
    if (out < 0.0) out += 360.0;
    return out;
}

double horizontalDistanceM(double lat1, double lon1, double lat2, double lon2)
{
    const double avgLatRad = qDegreesToRadians((lat1 + lat2) * 0.5);
    const double northM = (lat2 - lat1) * 111320.0;
    const double eastM = (lon2 - lon1) * 111320.0 * qMax(0.2, qCos(avgLatRad));
    return qSqrt(northM * northM + eastM * eastM);
}

double bearingDeg(double lat1, double lon1, double lat2, double lon2)
{
    const double phi1 = qDegreesToRadians(lat1);
    const double phi2 = qDegreesToRadians(lat2);
    const double dLambda = qDegreesToRadians(lon2 - lon1);
    const double y = qSin(dLambda) * qCos(phi2);
    const double x = qCos(phi1) * qSin(phi2)
                   - qSin(phi1) * qCos(phi2) * qCos(dLambda);
    return normalizeHeading(qRadiansToDegrees(qAtan2(y, x)));
}
} // namespace

MockVehicle::MockVehicle(gcs::vehicle::Vehicle* vehicle, QObject* parent)
    : QObject(parent),
      m_vehicle(vehicle),
      m_missionLink(std::make_unique<MockMissionLink>(this))
{
    Q_ASSERT(m_vehicle);

    auto* store = m_vehicle->stateStore();
    store->markSimulated(true);
    store->setLinkKind(gcs::comms::LinkKind::Mock); // Phase 7: stamp transport
    store->updateFlightMode(QStringLiteral("MANUAL"));
    store->updateArmed(false);
    store->updateBattery(m_voltage, m_percent);
    store->updateGps(3 /*3D fix*/, 12);
    store->updateLinkStatus(LinkStatus::Disconnected);

    m_heartbeatTimer.setInterval(1000); // 1 Hz
    m_telemetryTimer.setInterval(100);  // 10 Hz

    connect(&m_heartbeatTimer, &QTimer::timeout, this, &MockVehicle::emitHeartbeat);
    connect(&m_telemetryTimer, &QTimer::timeout, this, &MockVehicle::emitTelemetry);
}

MockVehicle::~MockVehicle() = default;

void MockVehicle::onManualControlSample(int16_t x, int16_t y, int16_t z, int16_t r,
                                        uint16_t /*buttons*/)
{
    m_lastX = x;
    m_lastY = y;
    m_lastZ = z;
    m_lastR = r;
    ++m_manualSampleCount;
    m_lastManualSampleUtcMs = QDateTime::currentMSecsSinceEpoch();
    // SAFETY: this is simulation-only. Samples never leave MockVehicle and
    // are used only to make mock telemetry visibly respond in the UI.
}

void MockVehicle::start()
{
    m_vehicle->appendEvent(QStringLiteral("[SIM] MockVehicle started"));
    m_heartbeatTimer.start();
    m_telemetryTimer.start();
    // Push one synchronous tick so the UI has data immediately.
    emitHeartbeat();
    emitTelemetry();
}

void MockVehicle::stop()
{
    stopMissionPreview(QStringLiteral("MockVehicle stopped"));
    m_heartbeatTimer.stop();
    m_telemetryTimer.stop();
    m_vehicle->stateStore()->updateLinkStatus(LinkStatus::Disconnected);
    m_vehicle->appendEvent(QStringLiteral("[SIM] MockVehicle stopped"));
}

bool MockVehicle::startMissionPreview(const gcs::mission::MissionPlan& plan,
                                      double speedMps)
{
    if (plan.items.isEmpty()) {
        return false;
    }

    m_previewPlan = plan;
    m_previewSpeedMps = clamp(speedMps > 0.0 ? speedMps : plan.cruiseSpeedMps,
                              1.0, 50.0);
    m_missionPreviewActive = true;
    m_previewTargetIndex = (plan.items.size() > 1) ? 1 : 0;
    m_previewProgress = 0.0;
    m_previewStatus = QStringLiteral("Running");

    const auto& first = plan.items.first();
    m_simLat = first.latitudeDeg;
    m_simLon = first.longitudeDeg;
    m_relAltM = qMax(0.0, first.altitudeM);
    if (std::isfinite(first.yawDeg)) {
        m_headingDeg = normalizeHeading(first.yawDeg);
    }

    auto* store = m_vehicle->stateStore();
    store->updateFlightMode(QStringLiteral("AUTO (PREVIEW)"));
    store->updateArmed(true);
    publishMissionPreviewState(m_simLat, m_simLon, m_relAltM, 0.0, m_headingDeg);

    m_vehicle->appendEvent(QStringLiteral("[SIM] Mission preview started (%1 items)")
                           .arg(plan.items.size()));
    emit missionPreviewStarted(plan.items.size());
    emit missionPreviewProgressChanged(m_previewTargetIndex, plan.items.size(),
                                       m_previewProgress);

    if (plan.items.size() == 1) {
        finishMissionPreview();
    }
    return true;
}

void MockVehicle::stopMissionPreview(const QString& reason)
{
    if (!m_missionPreviewActive) {
        return;
    }

    m_missionPreviewActive = false;
    m_previewStatus = QStringLiteral("Stopped");
    m_vehicle->stateStore()->updateFlightMode(QStringLiteral("MANUAL"));
    m_vehicle->stateStore()->updateArmed(false);
    m_vehicle->stateStore()->updateGroundSpeed(0.0);
    m_vehicle->appendEvent(QStringLiteral("[SIM] Mission preview stopped: %1")
                           .arg(reason));
    emit missionPreviewStopped(reason);
}

void MockVehicle::simulateLostHeartbeat(int durationMs)
{
    m_heartbeatSuppressed = true;
    m_vehicle->stateStore()->updateLinkStatus(LinkStatus::Stale);
    m_vehicle->appendEvent(
        QStringLiteral("[SIM] Heartbeat lost for %1 ms").arg(durationMs));

    QTimer::singleShot(durationMs, this, [this] {
        m_heartbeatSuppressed = false;
        m_vehicle->appendEvent(QStringLiteral("[SIM] Heartbeat restored"));
    });
}

void MockVehicle::simulateLowBattery()
{
    m_lowBatteryForced = true;
    m_percent = 15.0;
    m_voltage = 14.4;
    m_vehicle->stateStore()->updateBattery(m_voltage, m_percent);
    m_vehicle->appendEvent(QStringLiteral("[SIM] Low battery forced"));
}

void MockVehicle::setHeartbeatPeriodMs(int ms)
{
    m_heartbeatTimer.setInterval(ms);
}

void MockVehicle::setTelemetryPeriodMs(int ms)
{
    m_telemetryTimer.setInterval(ms);
}

void MockVehicle::tickHeartbeatNow()  { emitHeartbeat(); }
void MockVehicle::tickTelemetryNow()  { emitTelemetry(); }

void MockVehicle::publishMissionPreviewState(double lat, double lon, double altM,
                                             double speedMps, double headingDeg)
{
    m_simLat = lat;
    m_simLon = lon;
    m_relAltM = qMax(0.0, altM);
    m_headingDeg = normalizeHeading(headingDeg);
    m_groundSpeedMps = qMax(0.0, speedMps);

    auto* store = m_vehicle->stateStore();
    store->updatePosition(m_simLat, m_simLon, m_relAltM);
    store->updateAttitude(0.0, 0.0, m_headingDeg);
    store->updateGroundSpeed(m_groundSpeedMps);
}

void MockVehicle::tickMissionPreview(double dt)
{
    if (!m_missionPreviewActive || m_previewPlan.items.isEmpty()) {
        return;
    }

    if (m_previewTargetIndex < 0
        || m_previewTargetIndex >= m_previewPlan.items.size()) {
        finishMissionPreview();
        return;
    }

    const int itemCount = m_previewPlan.items.size();
    const auto& target = m_previewPlan.items.at(m_previewTargetIndex);
    if (!std::isfinite(target.latitudeDeg) || !std::isfinite(target.longitudeDeg)) {
        ++m_previewTargetIndex;
        return;
    }

    const double targetAlt = qMax(0.0, target.altitudeM);
    const double horizontalM =
        horizontalDistanceM(m_simLat, m_simLon,
                            target.latitudeDeg, target.longitudeDeg);
    const double altDelta = targetAlt - m_relAltM;
    const double distanceM = qSqrt(horizontalM * horizontalM + altDelta * altDelta);
    const double heading = horizontalM > 0.05
        ? bearingDeg(m_simLat, m_simLon, target.latitudeDeg, target.longitudeDeg)
        : m_headingDeg;

    auto publishProgress = [this, itemCount](double segmentFraction) {
        if (itemCount <= 1) {
            m_previewProgress = 1.0;
        } else {
            const double completed = qMax(0, m_previewTargetIndex - 1);
            m_previewProgress = clamp((completed + segmentFraction)
                                      / double(itemCount - 1),
                                      0.0, 1.0);
        }
        emit missionPreviewProgressChanged(m_previewTargetIndex, itemCount,
                                           m_previewProgress);
    };

    const double stepM = qMax(0.0, m_previewSpeedMps * dt);
    if (distanceM <= 0.2 || stepM >= distanceM) {
        publishMissionPreviewState(target.latitudeDeg, target.longitudeDeg,
                                   targetAlt, 0.0, heading);
        publishProgress(1.0);
        m_vehicle->appendEvent(QStringLiteral("[SIM] Mission preview reached item #%1")
                               .arg(target.seq + 1));
        emit missionPreviewWaypointReached(target.seq);

        if (m_previewTargetIndex >= itemCount - 1) {
            finishMissionPreview();
        } else {
            ++m_previewTargetIndex;
        }
        return;
    }

    const double f = stepM / distanceM;
    const double nextLat = m_simLat + (target.latitudeDeg - m_simLat) * f;
    const double nextLon = m_simLon + (target.longitudeDeg - m_simLon) * f;
    const double nextAlt = m_relAltM + (targetAlt - m_relAltM) * f;
    publishMissionPreviewState(nextLat, nextLon, nextAlt,
                               m_previewSpeedMps, heading);
    publishProgress(f);
}

void MockVehicle::finishMissionPreview()
{
    if (!m_missionPreviewActive) {
        return;
    }

    m_missionPreviewActive = false;
    m_previewProgress = 1.0;
    m_previewStatus = QStringLiteral("Completed");
    m_vehicle->stateStore()->updateFlightMode(QStringLiteral("MISSION COMPLETE"));
    m_vehicle->stateStore()->updateArmed(false);
    m_vehicle->stateStore()->updateGroundSpeed(0.0);
    m_vehicle->appendEvent(QStringLiteral("[SIM] Mission preview completed (%1 items)")
                           .arg(m_previewPlan.items.size()));
    emit missionPreviewProgressChanged(m_previewPlan.items.size() - 1,
                                       m_previewPlan.items.size(),
                                       m_previewProgress);
    emit missionPreviewCompleted(m_previewPlan.items.size());
}

void MockVehicle::emitHeartbeat()
{
    if (m_heartbeatSuppressed) {
        return;
    }
    auto* store = m_vehicle->stateStore();
    store->updateHeartbeat(
        m_vehicle->systemId(),
        m_vehicle->componentId(),
        QStringLiteral("quadrotor"),
        QStringLiteral("MockAutopilot"),
        QDateTime::currentMSecsSinceEpoch());
}

void MockVehicle::emitTelemetry()
{
    auto* store = m_vehicle->stateStore();

    ++m_tickCount;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const double dt = qMax(0.02, m_telemetryTimer.interval() / 1000.0);
    const bool manualFresh =
        m_lastManualSampleUtcMs > 0 && (now - m_lastManualSampleUtcMs) <= 750;

    if (m_missionPreviewActive) {
        tickMissionPreview(dt);
    } else if (manualFresh) {
        const double pitch   = clamp(m_lastX / 1000.0, -1.0, 1.0);
        const double roll    = clamp(m_lastY / 1000.0, -1.0, 1.0);
        const double climb   = clamp((m_lastZ - 500.0) / 500.0, -1.0, 1.0);
        const double yawRate = clamp(m_lastR / 1000.0, -1.0, 1.0);

        m_headingDeg = normalizeHeading(m_headingDeg + yawRate * 90.0 * dt);
        m_relAltM = clamp(m_relAltM + climb * 3.0 * dt, 0.0, 120.0);

        const double forwardMps = pitch * 8.0;
        const double sideMps    = roll * 4.0;
        const double headingRad = qDegreesToRadians(m_headingDeg);
        const double northMps =
            qCos(headingRad) * forwardMps - qSin(headingRad) * sideMps;
        const double eastMps =
            qSin(headingRad) * forwardMps + qCos(headingRad) * sideMps;

        m_simLat += (northMps * dt) / 111320.0;
        const double lonScale = 111320.0 * qMax(0.2, qCos(qDegreesToRadians(m_simLat)));
        m_simLon += (eastMps * dt) / lonScale;

        m_groundSpeedMps = qSqrt(northMps * northMps + eastMps * eastMps);
        store->updatePosition(m_simLat, m_simLon, m_relAltM);
        store->updateAttitude(roll * 30.0, pitch * 20.0, m_headingDeg);
        store->updateGroundSpeed(m_groundSpeedMps);
    } else {
    // Lazy little orbit so the UI shows motion. ~10 m radius at ~10 Hz.
    const double t = m_tickCount * 0.1; // seconds
    const double dLat = qSin(t * 0.1) * 0.00009;  // ~10 m
    const double dLon = qCos(t * 0.1) * 0.00009;
        m_simLat = m_baseLat + dLat;
        m_simLon = m_baseLon + dLon;
        store->updatePosition(m_simLat, m_simLon, m_relAltM);

        m_headingDeg = normalizeHeading(m_headingDeg + 1.0);
        store->updateAttitude(0.0, 0.0, m_headingDeg);

        m_groundSpeedMps = 3.0 + qSin(t * 0.5) * 0.5;
        store->updateGroundSpeed(m_groundSpeedMps);
    }

    // Slow battery decay (capped so the UI stays usable in long sessions).
    if (!m_lowBatteryForced) {
        m_percent = qMax(20.0, m_percent - 0.001);
        m_voltage = 14.0 + (m_percent / 100.0) * 2.8;
        store->updateBattery(m_voltage, m_percent);
    }
}

} // namespace gcs::simulation
