#include "Vehicle.h"

#include <QDateTime>
#include <QtMath>

#include "Firmware/FirmwarePlugin.h"
#include "Firmware/MavType.h"
#include "Protocol/MavlinkFrame.h"

namespace gcs::vehicle {

Vehicle::Vehicle(int systemId,
                 int componentId,
                 gcs::firmware::FirmwarePlugin* firmware,
                 QObject* parent)
    : QObject(parent),
      m_systemId(systemId),
      m_componentId(componentId),
      m_firmware(firmware),
      m_stateStore(std::make_unique<VehicleStateStore>(this))
{
    m_staleCheck.setInterval(1000);
    connect(&m_staleCheck, &QTimer::timeout, this, &Vehicle::onStaleCheckTick);
    m_staleCheck.start();
}

Vehicle::~Vehicle() = default;

void Vehicle::appendEvent(const QString& message)
{
    const QString stamped = QStringLiteral("[%1] %2")
        .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),
             message);
    m_events.append(stamped);
    while (m_events.size() > kMaxEvents) {
        m_events.removeFirst();
    }
    emit eventAppended(stamped);
}

void Vehicle::onStaleCheckTick()
{
    const auto& s = m_stateStore->state();
    if (s.lastHeartbeatUtcMs <= 0) return; // never received

    const qint64 age = QDateTime::currentMSecsSinceEpoch() - s.lastHeartbeatUtcMs;
    if (age >= kLostThreshMs) {
        if (s.linkStatus != LinkStatus::Disconnected) {
            const auto old = s.linkStatus;
            m_stateStore->updateLinkStatus(LinkStatus::Disconnected);
            appendEvent(QStringLiteral("Link lost (heartbeat %1 s old)")
                        .arg(age / 1000.0, 0, 'f', 1));
            emit linkStateChanged(old, LinkStatus::Disconnected, age);
        }
    } else if (age >= kStaleThreshMs) {
        if (s.linkStatus == LinkStatus::Connected) {
            m_stateStore->updateLinkStatus(LinkStatus::Stale);
            appendEvent(QStringLiteral("Heartbeat stale (%1 s)")
                        .arg(age / 1000.0, 0, 'f', 1));
            emit linkStateChanged(LinkStatus::Connected, LinkStatus::Stale, age);
        }
    }
}

// ---------- Telemetry handlers ----------

void Vehicle::handleHeartbeat(const gcs::protocol::msg::Heartbeat& hb)
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const auto& prevState = m_stateStore->state();
    const bool wasFirstHeartbeat = (prevState.lastHeartbeatUtcMs == 0);
    const auto prevLink   = prevState.linkStatus;
    const auto wasArmed   = prevState.armed;
    const bool armed = (hb.base_mode & 0x80) != 0; // MAV_MODE_FLAG_SAFETY_ARMED

    QString mode = QStringLiteral("UNKNOWN");
    if (m_firmware) {
        mode = m_firmware->decodeFlightMode(hb.base_mode, hb.custom_mode);
    }

    // Phase 5: derive the airframe label from HEARTBEAT.type so the UI can
    // distinguish Copter / Plane / Rover / Sub. Mock telemetry never sets
    // hb.type — in that case fall through to whatever the store currently
    // holds (set by MockVehicle to "quadrotor").
    const QString airframe = (hb.type != 0)
        ? gcs::firmware::airframeNameFromMavType(hb.type)
        : prevState.vehicleType;

    m_stateStore->updateHeartbeat(
        m_systemId, m_componentId,
        airframe,
        m_firmware ? m_firmware->firmwareName() : QStringLiteral("unknown"),
        now);
    m_stateStore->updateFlightMode(mode);
    m_stateStore->updateArmed(armed);

    // Phase 6: surface transitions for the EventLogger to record.
    if (wasFirstHeartbeat) {
        emit heartbeatReceivedFirstTime();
    } else if (prevLink != LinkStatus::Connected) {
        // Recovered from Stale/Disconnected → Connected.
        emit linkStateChanged(prevLink, LinkStatus::Connected, 0);
    }

    if (armed != wasArmed) {
        appendEvent(armed ? QStringLiteral("Vehicle ARMED")
                          : QStringLiteral("Vehicle DISARMED"));
    }
}

void Vehicle::handleSysStatus(const gcs::protocol::msg::SysStatus& s)
{
    const double voltage = s.voltage_battery_mV / 1000.0;
    double percent = -1.0;
    if (s.battery_remaining >= 0 && s.battery_remaining <= 100) {
        percent = double(s.battery_remaining);
    }
    m_stateStore->updateBattery(voltage, percent);
}

void Vehicle::handleGpsRawInt(const gcs::protocol::msg::GpsRawInt& g)
{
    m_stateStore->updateGps(int(g.fix_type), int(g.satellites_visible));
}

void Vehicle::handleAttitude(const gcs::protocol::msg::Attitude& a)
{
    const double rollDeg  = qRadiansToDegrees(double(a.roll_rad));
    const double pitchDeg = qRadiansToDegrees(double(a.pitch_rad));
    double yawDeg = qRadiansToDegrees(double(a.yaw_rad));
    // Normalize yaw to [0, 360).
    yawDeg = std::fmod(yawDeg + 360.0, 360.0);
    m_stateStore->updateAttitude(rollDeg, pitchDeg, yawDeg);
}

void Vehicle::handleGlobalPositionInt(const gcs::protocol::msg::GlobalPositionInt& p)
{
    const double lat = p.lat_e7 / 1.0e7;
    const double lon = p.lon_e7 / 1.0e7;
    const double relAlt = p.relative_alt_mm / 1000.0;
    m_stateStore->updatePosition(lat, lon, relAlt);
    if (p.hdg_cdeg != 65535) {
        m_stateStore->updateHeading(p.hdg_cdeg / 100.0);
    }
}

void Vehicle::handleVfrHud(const gcs::protocol::msg::VfrHud& v)
{
    m_stateStore->updateGroundSpeed(v.groundspeed_mps);
    // VFR_HUD also carries heading and alt — GLOBAL_POSITION_INT is the
    // authoritative source for those when available, so leave them alone here.
}

void Vehicle::handleBatteryStatus(const gcs::protocol::msg::BatteryStatus& b)
{
    if (b.battery_remaining >= 0 && b.battery_remaining <= 100) {
        const auto& s = m_stateStore->state();
        m_stateStore->updateBattery(s.batteryVoltage, double(b.battery_remaining));
    }
}

void Vehicle::handleStatustext(const gcs::protocol::msg::Statustext& t)
{
    if (t.text.isEmpty()) return;

    // Phase 5: surface ArduPilot's "PreArm:" / PX4's "Pre-arm" pre-arm
    // messages with their own tag so operators can spot them at a glance.
    const QString prefix = m_firmware ? m_firmware->prearmTextPrefix() : QString();
    const bool isPrearm = !prefix.isEmpty()
                          && t.text.startsWith(prefix, Qt::CaseInsensitive);
    const QString tag = isPrearm
        ? QStringLiteral("[PREARM sev=%1]").arg(t.severity)
        : QStringLiteral("[STATUS sev=%1]").arg(t.severity);
    appendEvent(QStringLiteral("%1 %2").arg(tag, t.text));

    // Phase 6: emit for the EventLogger to record under category=Vehicle.
    emit statustextReceived(int(t.severity), t.text, isPrearm);
}

} // namespace gcs::vehicle
