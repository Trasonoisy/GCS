#include <QtTest/QtTest>
#include <memory>

#include "Firmware/ArduCopterFirmwarePlugin.h"
#include "Firmware/ArduPilotFirmwarePlugin.h"
#include "Firmware/ArduPlaneFirmwarePlugin.h"
#include "Firmware/ArduRoverFirmwarePlugin.h"
#include "Firmware/ArduSubFirmwarePlugin.h"
#include "Firmware/FirmwarePluginManager.h"
#include "Firmware/MavType.h"
#include "Firmware/PX4FirmwarePlugin.h"

using namespace gcs::firmware;

class TestFirmwarePluginManager : public QObject
{
    Q_OBJECT
private slots:
    void px4AutopilotPicksPx4Plugin();
    void ardupilotQuadrotorPicksCopter();
    void ardupilotFixedWingPicksPlane();
    void ardupilotRoverPicksRover();
    void ardupilotSubPicksSub();
    void ardupilotUnknownAirframeFallsBackToBase();
    void unknownAutopilotFallsBackToBase();
};

namespace {
template <typename T>
bool isInstanceOf(QObject* obj)
{
    return qobject_cast<T*>(obj) != nullptr;
}
} // namespace

void TestFirmwarePluginManager::px4AutopilotPicksPx4Plugin()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        autopilot::Px4, /*mavType*/ 2, &parent);
    QVERIFY(p);
    QVERIFY(isInstanceOf<PX4FirmwarePlugin>(p));
}

void TestFirmwarePluginManager::ardupilotQuadrotorPicksCopter()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        autopilot::ArduPilotMega, /*MAV_TYPE_QUADROTOR*/ 2, &parent);
    QVERIFY(isInstanceOf<ArduCopterFirmwarePlugin>(p));
}

void TestFirmwarePluginManager::ardupilotFixedWingPicksPlane()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        autopilot::ArduPilotMega, /*MAV_TYPE_FIXED_WING*/ 1, &parent);
    QVERIFY(isInstanceOf<ArduPlaneFirmwarePlugin>(p));
}

void TestFirmwarePluginManager::ardupilotRoverPicksRover()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        autopilot::ArduPilotMega, /*MAV_TYPE_GROUND_ROVER*/ 10, &parent);
    QVERIFY(isInstanceOf<ArduRoverFirmwarePlugin>(p));
}

void TestFirmwarePluginManager::ardupilotSubPicksSub()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        autopilot::ArduPilotMega, /*MAV_TYPE_SUBMARINE*/ 12, &parent);
    QVERIFY(isInstanceOf<ArduSubFirmwarePlugin>(p));
}

void TestFirmwarePluginManager::ardupilotUnknownAirframeFallsBackToBase()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        autopilot::ArduPilotMega, /*unknown*/ 255, &parent);
    // Subclasses inherit from ArduPilotFirmwarePlugin, so cast to base is
    // ambiguous-free as long as we exclude the subclasses.
    QVERIFY(p);
    QVERIFY(!isInstanceOf<ArduCopterFirmwarePlugin>(p));
    QVERIFY(!isInstanceOf<ArduPlaneFirmwarePlugin>(p));
    QVERIFY(!isInstanceOf<ArduRoverFirmwarePlugin>(p));
    QVERIFY(!isInstanceOf<ArduSubFirmwarePlugin>(p));
    QVERIFY(isInstanceOf<ArduPilotFirmwarePlugin>(p));
}

void TestFirmwarePluginManager::unknownAutopilotFallsBackToBase()
{
    QObject parent;
    auto* p = FirmwarePluginManager::createForHeartbeat(
        /*MAV_AUTOPILOT_GENERIC*/ 0, /*MAV_TYPE_QUADROTOR*/ 2, &parent);
    QVERIFY(p);
    QVERIFY(!isInstanceOf<PX4FirmwarePlugin>(p));
    QVERIFY(isInstanceOf<ArduPilotFirmwarePlugin>(p));
}

QTEST_MAIN(TestFirmwarePluginManager)
#include "tst_firmware_plugin_manager.moc"
