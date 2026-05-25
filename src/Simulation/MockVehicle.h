#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

#include "Manual/IManualControlSink.h"

namespace gcs::vehicle { class Vehicle; }

namespace gcs::simulation {

class MockMissionLink;

// Generates fake telemetry for a Vehicle so the GCS can be brought up without
// any drone or SITL. Drives VehicleStateStore directly via the attached
// Vehicle. Does NOT push MAVLink bytes through a link — Phase 3 will replace
// this with a SITL-driven flow.
//
// SAFETY: MockVehicle ALWAYS marks its target state with simulated = true.
// The UI must display a "SIMULATION MODE" banner whenever any active vehicle
// has simulated == true.
//
// Phase 4: implements IManualControlSink so the ManualControlManager send
// loop can deliver MANUAL_CONTROL samples to the simulated vehicle.
class MockVehicle : public QObject, public gcs::manual::IManualControlSink
{
    Q_OBJECT
public:
    explicit MockVehicle(gcs::vehicle::Vehicle* vehicle, QObject* parent = nullptr);
    ~MockVehicle() override;

    // ---- IManualControlSink -----
    void onManualControlSample(int16_t x, int16_t y, int16_t z, int16_t r,
                               uint16_t buttons) override;
    QString sinkName() const override { return QStringLiteral("MockVehicle"); }
    bool    isSimulated() const override { return true; }

    int16_t  lastManualX() const { return m_lastX; }
    int16_t  lastManualY() const { return m_lastY; }
    int16_t  lastManualZ() const { return m_lastZ; }
    int16_t  lastManualR() const { return m_lastR; }
    int      manualSampleCount() const { return m_manualSampleCount; }

    // The simulated mission-protocol peer. Wire this into MissionManager so
    // the GCS-side upload/download state machines have someone to talk to.
    MockMissionLink* missionLink() const { return m_missionLink.get(); }

    gcs::vehicle::Vehicle* vehicle() const { return m_vehicle; }

    void start();
    void stop();
    bool isRunning() const { return m_telemetryTimer.isActive(); }

    // Test hooks
    void simulateLostHeartbeat(int durationMs);
    void simulateLowBattery();
    void setHeartbeatPeriodMs(int ms);
    void setTelemetryPeriodMs(int ms);

    // Force one tick — used by tests to avoid waiting on the timer.
    void tickHeartbeatNow();
    void tickTelemetryNow();

private:
    void emitHeartbeat();
    void emitTelemetry();

    gcs::vehicle::Vehicle* m_vehicle;
    std::unique_ptr<MockMissionLink> m_missionLink;
    QTimer m_heartbeatTimer;
    QTimer m_telemetryTimer;
    bool   m_heartbeatSuppressed = false;
    bool   m_lowBatteryForced    = false;

    // Synthetic flight state
    double m_baseLat        = 21.028511;  // Hanoi-ish, just for demo
    double m_baseLon        = 105.804817;
    double m_relAltM        = 0.0;
    double m_headingDeg     = 0.0;
    double m_groundSpeedMps = 0.0;
    double m_voltage        = 16.8;
    double m_percent        = 100.0;
    int    m_tickCount      = 0;

    // Last received manual-control sample (Phase 4).
    int16_t m_lastX = 0, m_lastY = 0, m_lastZ = 500, m_lastR = 0;
    int     m_manualSampleCount = 0;
};

} // namespace gcs::simulation
