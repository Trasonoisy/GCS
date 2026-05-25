#pragma once

#include <QObject>
#include <QString>

#include "VehicleState.h"

namespace gcs::vehicle {

// Owns one VehicleState snapshot and emits stateChanged() whenever any field
// is updated. ViewModels listen here. No QML imports allowed.
class VehicleStateStore : public QObject
{
    Q_OBJECT
public:
    explicit VehicleStateStore(QObject* parent = nullptr);

    const VehicleState& state() const { return m_state; }

    // Bulk update — emits one signal at the end if anything changed.
    void applyState(const VehicleState& next);

    // Targeted updates used by simulation and (later) protocol decoders.
    void updateHeartbeat(int systemId,
                         int componentId,
                         const QString& vehicleType,
                         const QString& autopilotType,
                         qint64 utcMs);
    void updateFlightMode(const QString& mode);
    void updateArmed(bool armed);
    void updatePosition(double latDeg, double lonDeg, double relAltM);
    void updateHeading(double headingDeg);
    void updateAttitude(double rollDeg, double pitchDeg, double yawDeg);
    void updateGroundSpeed(double mps);
    void updateBattery(double voltage, double percent);
    void updateGps(int fixType, int satellitesVisible);
    void updateLinkStatus(LinkStatus status);
    void setLinkKind(gcs::comms::LinkKind kind);
    void markSimulated(bool simulated);

signals:
    void stateChanged();

private:
    VehicleState m_state;
};

} // namespace gcs::vehicle
