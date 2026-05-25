#pragma once

#include <QList>
#include <QObject>

namespace gcs::vehicle {

class Vehicle;

// Tracks all known vehicles and exposes one "active" vehicle to the UI.
// Phase 1 supports exactly one vehicle (the simulated mock), but the API is
// shaped so multi-vehicle phases drop in without UI rework.
class MultiVehicleManager : public QObject
{
    Q_OBJECT
public:
    explicit MultiVehicleManager(QObject* parent = nullptr);
    ~MultiVehicleManager() override;

    // Takes ownership of `vehicle` via Qt parent/child.
    void addVehicle(Vehicle* vehicle);
    void removeVehicle(Vehicle* vehicle);

    const QList<Vehicle*>& vehicles() const { return m_vehicles; }
    Vehicle* activeVehicle() const { return m_activeVehicle; }
    void setActiveVehicle(Vehicle* vehicle);

    // Returns nullptr if no vehicle with this identity exists yet.
    Vehicle* findBySysCompId(int systemId, int componentId) const;

signals:
    void vehicleAdded(Vehicle* vehicle);
    void vehicleRemoved(Vehicle* vehicle);
    void activeVehicleChanged(Vehicle* vehicle);

private:
    QList<Vehicle*> m_vehicles;
    Vehicle* m_activeVehicle = nullptr;
};

} // namespace gcs::vehicle
