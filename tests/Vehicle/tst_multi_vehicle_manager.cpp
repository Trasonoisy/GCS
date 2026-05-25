#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"
#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"

using gcs::firmware::PX4FirmwarePlugin;
using gcs::vehicle::MultiVehicleManager;
using gcs::vehicle::Vehicle;

class TestMultiVehicleManager : public QObject
{
    Q_OBJECT
private slots:
    void exposesFirstVehicleAsActive();
    void emitsAddedAndActiveChanged();
    void promotesNextVehicleWhenActiveRemoved();
};

void TestMultiVehicleManager::exposesFirstVehicleAsActive()
{
    MultiVehicleManager manager;
    QVERIFY(manager.activeVehicle() == nullptr);

    auto* fw = new PX4FirmwarePlugin(&manager);
    auto* v = new Vehicle(1, 1, fw);
    manager.addVehicle(v);

    QCOMPARE(manager.activeVehicle(), v);
    QCOMPARE(manager.vehicles().size(), 1);
}

void TestMultiVehicleManager::emitsAddedAndActiveChanged()
{
    MultiVehicleManager manager;
    QSignalSpy addedSpy(&manager, &MultiVehicleManager::vehicleAdded);
    QSignalSpy activeSpy(&manager, &MultiVehicleManager::activeVehicleChanged);

    auto* fw = new PX4FirmwarePlugin(&manager);
    auto* v = new Vehicle(1, 1, fw);
    manager.addVehicle(v);

    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(activeSpy.count(), 1);
}

void TestMultiVehicleManager::promotesNextVehicleWhenActiveRemoved()
{
    MultiVehicleManager manager;
    auto* fw = new PX4FirmwarePlugin(&manager);
    auto* v1 = new Vehicle(1, 1, fw);
    auto* v2 = new Vehicle(2, 1, fw);
    manager.addVehicle(v1);
    manager.addVehicle(v2);
    QCOMPARE(manager.activeVehicle(), v1);

    manager.removeVehicle(v1);
    QCOMPARE(manager.activeVehicle(), v2);

    manager.removeVehicle(v2);
    QVERIFY(manager.activeVehicle() == nullptr);
}

QTEST_MAIN(TestMultiVehicleManager)
#include "tst_multi_vehicle_manager.moc"
