#include "MultiVehicleManager.h"

#include "Vehicle.h"

namespace gcs::vehicle {

MultiVehicleManager::MultiVehicleManager(QObject* parent) : QObject(parent) {}

MultiVehicleManager::~MultiVehicleManager() = default;

void MultiVehicleManager::addVehicle(Vehicle* vehicle)
{
    if (!vehicle || m_vehicles.contains(vehicle)) {
        return;
    }
    vehicle->setParent(this);
    m_vehicles.append(vehicle);
    emit vehicleAdded(vehicle);

    if (!m_activeVehicle) {
        setActiveVehicle(vehicle);
    }
}

void MultiVehicleManager::removeVehicle(Vehicle* vehicle)
{
    if (!vehicle || !m_vehicles.contains(vehicle)) {
        return;
    }
    m_vehicles.removeAll(vehicle);
    emit vehicleRemoved(vehicle);

    if (m_activeVehicle == vehicle) {
        setActiveVehicle(m_vehicles.isEmpty() ? nullptr : m_vehicles.first());
    }
}

void MultiVehicleManager::setActiveVehicle(Vehicle* vehicle)
{
    if (m_activeVehicle == vehicle) return;
    m_activeVehicle = vehicle;
    emit activeVehicleChanged(vehicle);
}

Vehicle* MultiVehicleManager::findBySysCompId(int systemId, int componentId) const
{
    for (auto* v : m_vehicles) {
        if (v->systemId() == systemId && v->componentId() == componentId) return v;
    }
    return nullptr;
}

} // namespace gcs::vehicle
