#include <QDateTime>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"
#include "Manual/ManualControlManager.h"
#include "Manual/ManualControlState.h"
#include "Manual/MockJoystickBackend.h"
#include "Safety/SafetyGate.h"
#include "Simulation/MockVehicle.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

using gcs::firmware::PX4FirmwarePlugin;
using gcs::manual::ManualControlManager;
using gcs::manual::ManualControlState;
using gcs::manual::MockJoystickBackend;
using gcs::safety::SafetyGate;
using gcs::simulation::MockVehicle;
using gcs::vehicle::LinkStatus;
using gcs::vehicle::Vehicle;

namespace {

struct Fixture
{
    PX4FirmwarePlugin fw;
    Vehicle           veh;
    MockVehicle       mock;
    MockJoystickBackend joystick;
    SafetyGate        gate;
    ManualControlManager manager;

    Fixture()
        : veh(200, 1, &fw),
          mock(&veh),
          manager(&joystick, &gate)
    {
        // Identity axis processing so tests can reason about raw->packed
        // values directly. The default has deadzone=0.05 + expo=0.30.
        for (auto* cfg : {&joystick.pitchConfig(), &joystick.rollConfig(),
                          &joystick.throttleConfig(), &joystick.yawConfig()}) {
            cfg->deadzone = 0.0;
            cfg->expo     = 0.0;
        }
        manager.setActiveVehicle(&veh);
        manager.setSink(&mock);
        // Force the state store into "alive" by pushing a heartbeat from the
        // MockVehicle helper.
        mock.tickHeartbeatNow();
    }
};

void freshHeartbeat(Fixture& f)
{
    f.veh.stateStore()->updateHeartbeat(
        f.veh.systemId(), f.veh.componentId(),
        QStringLiteral("quadrotor"), QStringLiteral("PX4"),
        QDateTime::currentMSecsSinceEpoch());
}

} // namespace

class TestManualControlManager : public QObject
{
    Q_OBJECT
private slots:
    void startsDisabled();
    void enableWithoutJoystickGoesWaiting();
    void enableWithEverythingGoesActive();
    void disableStopsTheSendLoop();
    void joystickDisconnectStopsActive();
    void staleHeartbeatStopsActive();
    void changingActiveVehicleStopsActive();
    void sendLoopPushesSamplesToSink();
    void ardupilotSitlChecklistMatchesGate();
};

void TestManualControlManager::startsDisabled()
{
    Fixture f;
    QCOMPARE(f.manager.state(), ManualControlState::Disabled);
}

void TestManualControlManager::enableWithoutJoystickGoesWaiting()
{
    Fixture f;
    freshHeartbeat(f);
    QVERIFY(!f.joystick.isConnected());
    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::WaitingForJoystick);
}

void TestManualControlManager::enableWithEverythingGoesActive()
{
    Fixture f;
    freshHeartbeat(f);
    f.joystick.setConnected(true);
    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);
}

void TestManualControlManager::disableStopsTheSendLoop()
{
    Fixture f;
    freshHeartbeat(f);
    f.joystick.setConnected(true);
    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);

    f.manager.disable();
    QCOMPARE(f.manager.state(), ManualControlState::Disabled);
}

void TestManualControlManager::joystickDisconnectStopsActive()
{
    Fixture f;
    freshHeartbeat(f);
    f.joystick.setConnected(true);
    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);

    f.joystick.setConnected(false);
    // Goes to WaitingForJoystick when active session is torn down by the
    // watchdog and operator is still opted in.
    QVERIFY(f.manager.state() == ManualControlState::WaitingForJoystick
            || f.manager.state() == ManualControlState::Failsafe);
    QVERIFY(f.manager.state() != ManualControlState::Active);
}

void TestManualControlManager::staleHeartbeatStopsActive()
{
    Fixture f;
    freshHeartbeat(f);
    f.joystick.setConnected(true);
    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);

    // Backdate the heartbeat so canContinueManualControl trips.
    f.veh.stateStore()->updateHeartbeat(
        f.veh.systemId(), f.veh.componentId(),
        QStringLiteral("quadrotor"), QStringLiteral("PX4"),
        QDateTime::currentMSecsSinceEpoch() - 10'000);
    QCOMPARE(f.manager.state(), ManualControlState::Failsafe);
}

void TestManualControlManager::changingActiveVehicleStopsActive()
{
    Fixture f;
    freshHeartbeat(f);
    f.joystick.setConnected(true);
    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);

    PX4FirmwarePlugin fw2;
    Vehicle other(1, 1, &fw2);
    f.manager.setActiveVehicle(&other);
    QVERIFY(f.manager.state() != ManualControlState::Active);
}

void TestManualControlManager::sendLoopPushesSamplesToSink()
{
    Fixture f;
    freshHeartbeat(f);
    f.joystick.setConnected(true);
    f.joystick.setRawPitch(0.5);
    f.joystick.setRawThrottle(0.2);
    f.manager.setSendRateHz(50);
    QSignalSpy spy(&f.manager, &ManualControlManager::sampleSent);

    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);

    QVERIFY(spy.wait(500));
    QVERIFY(f.mock.manualSampleCount() >= 1);
    // pitch=0.5 packs to ~500
    QVERIFY(qAbs(f.mock.lastManualX() - 500) <= 5);
    // throttle=0.2 packs to (0.2+1)*500 = 600
    QVERIFY(qAbs(f.mock.lastManualZ() - 600) <= 5);
}

void TestManualControlManager::ardupilotSitlChecklistMatchesGate()
{
    Fixture f;
    f.veh.stateStore()->markSimulated(false);
    f.veh.stateStore()->setLinkKind(gcs::comms::LinkKind::Tcp);
    f.veh.stateStore()->updateHeartbeat(
        f.veh.systemId(), f.veh.componentId(),
        QStringLiteral("Copter"), QStringLiteral("ArduPilot"),
        QDateTime::currentMSecsSinceEpoch());
    f.joystick.setConnected(true);

    f.manager.enable();
    QCOMPARE(f.manager.state(), ManualControlState::Active);
    QVERIFY(f.manager.checklist().contains(
        QStringLiteral("[OK] Simulation or SITL")));
}

QTEST_MAIN(TestManualControlManager)
#include "tst_manual_control_manager.moc"
