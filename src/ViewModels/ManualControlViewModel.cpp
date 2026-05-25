#include "ManualControlViewModel.h"

#include "Manual/IManualControlSink.h"
#include "Manual/JoystickState.h"
#include "Manual/ManualControlManager.h"
#include "Manual/ManualControlState.h"
#include "Manual/MockJoystickBackend.h"
#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

namespace gcs::viewmodels {

using gcs::manual::ManualControlManager;
using gcs::manual::ManualControlState;
using gcs::manual::MockJoystickBackend;
using gcs::manual::manualControlStateName;
using gcs::vehicle::MultiVehicleManager;
using gcs::vehicle::Vehicle;

ManualControlViewModel::ManualControlViewModel(
    ManualControlManager* manager,
    MockJoystickBackend* joystick,
    MultiVehicleManager* vehicleManager,
    QObject* parent)
    : QObject(parent), m_manager(manager), m_joystick(joystick),
      m_vehicleMgr(vehicleManager)
{
    Q_ASSERT(m_manager && m_joystick && m_vehicleMgr);

    connect(m_manager, &ManualControlManager::stateChanged,
            this, &ManualControlViewModel::stateChanged);
    connect(m_manager, &ManualControlManager::checklistChanged,
            this, &ManualControlViewModel::stateChanged);
    connect(m_manager, &ManualControlManager::blockedReasonChanged,
            this, &ManualControlViewModel::stateChanged);
    connect(m_manager, &ManualControlManager::sampleSent,
            this, [this](int, int, int, int) { emit sampleSent(); });

    connect(m_joystick, &gcs::manual::JoystickBackend::stateChanged,
            this, &ManualControlViewModel::joystickChanged);

    connect(m_vehicleMgr, &MultiVehicleManager::activeVehicleChanged,
            this, &ManualControlViewModel::onActiveVehicleChanged);
}

QString ManualControlViewModel::stateName() const
{
    return QString::fromLatin1(manualControlStateName(m_manager->state()));
}
QString ManualControlViewModel::blockedReason() const { return m_manager->blockedReason(); }
QStringList ManualControlViewModel::checklist() const { return m_manager->checklist(); }
bool ManualControlViewModel::isActive() const
{ return m_manager->state() == ManualControlState::Active; }

bool ManualControlViewModel::joystickConnected() const { return m_joystick->isConnected(); }
QString ManualControlViewModel::joystickName() const   { return m_joystick->state().name; }

double ManualControlViewModel::rawPitch()    const { return m_joystick->rawPitch(); }
double ManualControlViewModel::rawRoll()     const { return m_joystick->rawRoll(); }
double ManualControlViewModel::rawThrottle() const { return m_joystick->rawThrottle(); }
double ManualControlViewModel::rawYaw()      const { return m_joystick->rawYaw(); }

double ManualControlViewModel::pitch()    const { return m_joystick->state().pitch; }
double ManualControlViewModel::roll()     const { return m_joystick->state().roll; }
double ManualControlViewModel::throttle() const { return m_joystick->state().throttle; }
double ManualControlViewModel::yaw()      const { return m_joystick->state().yaw; }

int ManualControlViewModel::lastX() const { return m_manager->lastX(); }
int ManualControlViewModel::lastY() const { return m_manager->lastY(); }
int ManualControlViewModel::lastZ() const { return m_manager->lastZ(); }
int ManualControlViewModel::lastR() const { return m_manager->lastR(); }
int ManualControlViewModel::totalSamplesSent() const { return m_manager->totalSamplesSent(); }

QString ManualControlViewModel::vehicleLabel() const
{
    if (auto* v = m_manager->activeVehicle()) {
        const auto& s = v->stateStore()->state();
        return QStringLiteral("%1 #%2").arg(s.vehicleType).arg(s.systemId);
    }
    return QStringLiteral("(no vehicle)");
}

QString ManualControlViewModel::sinkLabel() const
{
    if (auto* s = m_manager->sink()) return s->sinkName();
    return QStringLiteral("(no sink)");
}

bool ManualControlViewModel::sinkSimulated() const
{
    if (auto* s = m_manager->sink()) return s->isSimulated();
    return false;
}

void ManualControlViewModel::setRawPitch(double v)
{
    m_joystick->setRawPitch(v);
}
void ManualControlViewModel::setRawRoll(double v)     { m_joystick->setRawRoll(v); }
void ManualControlViewModel::setRawThrottle(double v) { m_joystick->setRawThrottle(v); }
void ManualControlViewModel::setRawYaw(double v)      { m_joystick->setRawYaw(v); }

void ManualControlViewModel::enable()
{
    m_operatorEnabled = true;
    m_manager->enable();
    emit stateChanged();
}

void ManualControlViewModel::disable()
{
    m_operatorEnabled = false;
    m_manager->disable();
    emit stateChanged();
}

void ManualControlViewModel::setJoystickConnected(bool c)
{
    m_joystick->setConnected(c);
}

void ManualControlViewModel::centreAxes()
{
    m_joystick->setRawPitch(0.0);
    m_joystick->setRawRoll(0.0);
    m_joystick->setRawThrottle(0.0);
    m_joystick->setRawYaw(0.0);
}

void ManualControlViewModel::onActiveVehicleChanged(Vehicle* /*v*/)
{
    // Sink rebinding happens in main.cpp's wiring (where we know the
    // sink instances). We just refresh property-driven labels.
    m_operatorEnabled = false;
    emit vehicleChanged();
    emit stateChanged();
}

} // namespace gcs::viewmodels
