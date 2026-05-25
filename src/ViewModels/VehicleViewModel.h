#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

namespace gcs::vehicle {
class MultiVehicleManager;
class Vehicle;
}

namespace gcs::viewmodels {

// QML-facing adapter for the active vehicle. Pure Q_PROPERTY / NOTIFY surface.
// Re-emits when the active vehicle's state store changes.
//
// Exposed to QML via QQmlContext::setContextProperty("vehicleVm", ...) in
// main.cpp — QML must talk only to this view model, never to backend types
// directly, never to MAVLink, never to safety-relevant services without going
// through SafetyGate (Phase 4+).
class VehicleViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool    hasVehicle          READ hasVehicle          NOTIFY changed)
    Q_PROPERTY(bool    simulated           READ simulated           NOTIFY changed)
    Q_PROPERTY(QString vehicleLabel        READ vehicleLabel        NOTIFY changed)
    Q_PROPERTY(int     systemId            READ systemId            NOTIFY changed)
    Q_PROPERTY(int     componentId         READ componentId         NOTIFY changed)
    Q_PROPERTY(QString vehicleType         READ vehicleType         NOTIFY changed)
    Q_PROPERTY(QString autopilotType       READ autopilotType       NOTIFY changed)
    Q_PROPERTY(QString flightMode          READ flightMode          NOTIFY changed)
    Q_PROPERTY(bool    armed               READ armed               NOTIFY changed)
    Q_PROPERTY(double  latitudeDeg         READ latitudeDeg         NOTIFY changed)
    Q_PROPERTY(double  longitudeDeg        READ longitudeDeg        NOTIFY changed)
    Q_PROPERTY(double  relativeAltitudeM   READ relativeAltitudeM   NOTIFY changed)
    Q_PROPERTY(double  headingDeg          READ headingDeg          NOTIFY changed)
    Q_PROPERTY(double  rollDeg             READ rollDeg             NOTIFY changed)
    Q_PROPERTY(double  pitchDeg            READ pitchDeg            NOTIFY changed)
    Q_PROPERTY(double  groundSpeedMps      READ groundSpeedMps      NOTIFY changed)
    Q_PROPERTY(double  batteryVoltage      READ batteryVoltage      NOTIFY changed)
    Q_PROPERTY(double  batteryPercent      READ batteryPercent      NOTIFY changed)
    Q_PROPERTY(int     gpsFixType          READ gpsFixType          NOTIFY changed)
    Q_PROPERTY(int     satellitesVisible   READ satellitesVisible   NOTIFY changed)
    Q_PROPERTY(QString linkStatusText      READ linkStatusText      NOTIFY changed)
    Q_PROPERTY(qint64  heartbeatAgeMs      READ heartbeatAgeMs      NOTIFY heartbeatAgeChanged)
    Q_PROPERTY(QStringList eventLog        READ eventLog            NOTIFY eventLogChanged)

public:
    explicit VehicleViewModel(gcs::vehicle::MultiVehicleManager* manager,
                              QObject* parent = nullptr);

    bool    hasVehicle()        const { return m_vehicle != nullptr; }
    bool    simulated()         const;
    QString vehicleLabel()      const;
    int     systemId()          const;
    int     componentId()       const;
    QString vehicleType()       const;
    QString autopilotType()     const;
    QString flightMode()        const;
    bool    armed()             const;
    double  latitudeDeg()       const;
    double  longitudeDeg()      const;
    double  relativeAltitudeM() const;
    double  headingDeg()        const;
    double  rollDeg()           const;
    double  pitchDeg()          const;
    double  groundSpeedMps()    const;
    double  batteryVoltage()    const;
    double  batteryPercent()    const;
    int     gpsFixType()        const;
    int     satellitesVisible() const;
    QString linkStatusText()    const;
    qint64  heartbeatAgeMs()    const;
    QStringList eventLog()      const;

signals:
    void changed();
    void heartbeatAgeChanged();
    void eventLogChanged();

private slots:
    void onActiveVehicleChanged(gcs::vehicle::Vehicle* vehicle);
    void onVehicleEventAppended(const QString& message);

private:
    void rebind(gcs::vehicle::Vehicle* vehicle);

    gcs::vehicle::MultiVehicleManager* m_manager;
    gcs::vehicle::Vehicle* m_vehicle = nullptr;
    QStringList m_events;
    QTimer m_heartbeatAgeTimer;
    static constexpr int kMaxUiEvents = 200;
};

} // namespace gcs::viewmodels
