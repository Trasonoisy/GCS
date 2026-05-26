#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantList>

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
    // Trail of recent positions for the map (oldest first). Each entry is a
    // QVariantMap with keys "lat" / "lon". Bounded to kMaxTrailPoints.
    Q_PROPERTY(QVariantList trail          READ trail               NOTIFY trailChanged)
    // True only when latitude/longitude are usable for map display — guards
    // the map from snapping to (0,0) before the first GPS fix.
    Q_PROPERTY(bool         hasValidPosition READ hasValidPosition  NOTIFY changed)

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
    QVariantList trail()        const { return m_trail; }
    bool        hasValidPosition() const;

    Q_INVOKABLE void clearTrail();

signals:
    void changed();
    void heartbeatAgeChanged();
    void eventLogChanged();
    void trailChanged();

private slots:
    void onActiveVehicleChanged(gcs::vehicle::Vehicle* vehicle);
    void onVehicleEventAppended(const QString& message);
    void onVehicleStateChanged();

private:
    void rebind(gcs::vehicle::Vehicle* vehicle);
    void appendTrailPointIfMoved(double lat, double lon);

    gcs::vehicle::MultiVehicleManager* m_manager;
    gcs::vehicle::Vehicle* m_vehicle = nullptr;
    QStringList m_events;
    QVariantList m_trail;
    double m_lastTrailLat = 0.0;
    double m_lastTrailLon = 0.0;
    bool   m_haveLastTrailPoint = false;
    QTimer m_heartbeatAgeTimer;
    static constexpr int kMaxUiEvents   = 200;
    static constexpr int kMaxTrailPoints = 2000;
    // Don't record a new trail point unless the vehicle moved at least this
    // many degrees from the last recorded point — keeps memory bounded for
    // a stationary mock vehicle while still preserving curved paths.
    static constexpr double kMinTrailDeltaDeg = 1e-5; // ~1.1 m at equator
};

} // namespace gcs::viewmodels
