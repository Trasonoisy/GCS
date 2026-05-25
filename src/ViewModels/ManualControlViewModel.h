#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace gcs::manual {
class ManualControlManager;
class MockJoystickBackend;
}
namespace gcs::vehicle {
class MultiVehicleManager;
class Vehicle;
}

namespace gcs::viewmodels {

// QML adapter over ManualControlManager + MockJoystickBackend. Exposes raw
// joystick axes (UI sliders write here), processed axes, packed MAVLink
// values, manager state, checklist, and the blocked reason. Drives sink
// selection on active-vehicle change.
class ManualControlViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString stateName        READ stateName        NOTIFY stateChanged)
    Q_PROPERTY(QString blockedReason    READ blockedReason    NOTIFY stateChanged)
    Q_PROPERTY(QStringList checklist    READ checklist        NOTIFY stateChanged)
    Q_PROPERTY(bool active              READ isActive         NOTIFY stateChanged)
    Q_PROPERTY(bool operatorEnabled     READ operatorEnabled  NOTIFY stateChanged)

    Q_PROPERTY(bool joystickConnected   READ joystickConnected NOTIFY joystickChanged)
    Q_PROPERTY(QString joystickName     READ joystickName      NOTIFY joystickChanged)

    Q_PROPERTY(double rawPitch          READ rawPitch    WRITE setRawPitch    NOTIFY joystickChanged)
    Q_PROPERTY(double rawRoll           READ rawRoll     WRITE setRawRoll     NOTIFY joystickChanged)
    Q_PROPERTY(double rawThrottle       READ rawThrottle WRITE setRawThrottle NOTIFY joystickChanged)
    Q_PROPERTY(double rawYaw            READ rawYaw      WRITE setRawYaw      NOTIFY joystickChanged)

    Q_PROPERTY(double pitch             READ pitch    NOTIFY joystickChanged)
    Q_PROPERTY(double roll              READ roll     NOTIFY joystickChanged)
    Q_PROPERTY(double throttle          READ throttle NOTIFY joystickChanged)
    Q_PROPERTY(double yaw               READ yaw      NOTIFY joystickChanged)

    Q_PROPERTY(int lastX                READ lastX NOTIFY sampleSent)
    Q_PROPERTY(int lastY                READ lastY NOTIFY sampleSent)
    Q_PROPERTY(int lastZ                READ lastZ NOTIFY sampleSent)
    Q_PROPERTY(int lastR                READ lastR NOTIFY sampleSent)
    Q_PROPERTY(int totalSamplesSent     READ totalSamplesSent NOTIFY sampleSent)

    Q_PROPERTY(QString vehicleLabel     READ vehicleLabel    NOTIFY vehicleChanged)
    Q_PROPERTY(QString sinkLabel        READ sinkLabel       NOTIFY vehicleChanged)
    Q_PROPERTY(bool    sinkSimulated    READ sinkSimulated   NOTIFY vehicleChanged)

public:
    ManualControlViewModel(gcs::manual::ManualControlManager* manager,
                           gcs::manual::MockJoystickBackend* joystick,
                           gcs::vehicle::MultiVehicleManager* vehicleManager,
                           QObject* parent = nullptr);

    QString     stateName()       const;
    QString     blockedReason()   const;
    QStringList checklist()       const;
    bool        isActive()        const;
    bool        operatorEnabled() const { return m_operatorEnabled; }

    bool    joystickConnected() const;
    QString joystickName()      const;

    double rawPitch()    const;
    double rawRoll()     const;
    double rawThrottle() const;
    double rawYaw()      const;
    double pitch()    const;
    double roll()     const;
    double throttle() const;
    double yaw()      const;

    int lastX() const;
    int lastY() const;
    int lastZ() const;
    int lastR() const;
    int totalSamplesSent() const;

    QString vehicleLabel() const;
    QString sinkLabel()    const;
    bool    sinkSimulated() const;

    void setRawPitch   (double v);
    void setRawRoll    (double v);
    void setRawThrottle(double v);
    void setRawYaw     (double v);

    Q_INVOKABLE void enable();
    Q_INVOKABLE void disable();
    Q_INVOKABLE void setJoystickConnected(bool c);
    Q_INVOKABLE void centreAxes();

signals:
    void stateChanged();
    void joystickChanged();
    void sampleSent();
    void vehicleChanged();

private slots:
    void onActiveVehicleChanged(gcs::vehicle::Vehicle* v);

private:
    gcs::manual::ManualControlManager* m_manager;
    gcs::manual::MockJoystickBackend*  m_joystick;
    gcs::vehicle::MultiVehicleManager* m_vehicleMgr;
    bool m_operatorEnabled = false;
};

} // namespace gcs::viewmodels
