#include "VehicleViewModel.h"

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

    if (m_vehicle) {
        connect(m_vehicle->stateStore(),
                &gcs::vehicle::VehicleStateStore::stateChanged,
                this, &VehicleViewModel::changed);
        connect(m_vehicle, &Vehicle::eventAppended,
                this, &VehicleViewModel::onVehicleEventAppended);

        // Backfill any events the vehicle already collected.
        for (const auto& e : m_vehicle->events()) {
            m_events.append(e);
        }
    }

    emit changed();
    emit eventLogChanged();
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
