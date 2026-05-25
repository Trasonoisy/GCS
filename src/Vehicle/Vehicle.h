#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <memory>

#include "VehicleStateStore.h"

namespace gcs::firmware { class FirmwarePlugin; }
namespace gcs::mission  { class MissionManager; }
namespace gcs::protocol { namespace msg {
    struct Heartbeat;
    struct SysStatus;
    struct GpsRawInt;
    struct Attitude;
    struct GlobalPositionInt;
    struct VfrHud;
    struct BatteryStatus;
    struct Statustext;
}}

namespace gcs::vehicle {

// Owns one Vehicle: its state store, firmware plugin, event log, and (later)
// command queue, mission manager, manual-control manager, parameter manager.
//
// Phase 3 adds typed handle*() entry points for MAVLink-derived telemetry
// plus a 1 Hz stale-heartbeat watcher that demotes link status as the
// vehicle goes silent.
class Vehicle : public QObject
{
    Q_OBJECT
public:
    explicit Vehicle(int systemId,
                     int componentId,
                     gcs::firmware::FirmwarePlugin* firmware,
                     QObject* parent = nullptr);
    ~Vehicle() override;

    int systemId() const { return m_systemId; }
    int componentId() const { return m_componentId; }

    VehicleStateStore* stateStore() const { return m_stateStore.get(); }
    gcs::firmware::FirmwarePlugin* firmwarePlugin() const { return m_firmware; }

    void setMissionManager(gcs::mission::MissionManager* m) { m_mission = m; }
    gcs::mission::MissionManager* missionManager() const { return m_mission; }

    void appendEvent(const QString& message);
    const QStringList& events() const { return m_events; }

    // Telemetry entry points — called by MAVLinkMessageRouter.
    void handleHeartbeat        (const gcs::protocol::msg::Heartbeat& hb);
    void handleSysStatus        (const gcs::protocol::msg::SysStatus& s);
    void handleGpsRawInt        (const gcs::protocol::msg::GpsRawInt& g);
    void handleAttitude         (const gcs::protocol::msg::Attitude& a);
    void handleGlobalPositionInt(const gcs::protocol::msg::GlobalPositionInt& p);
    void handleVfrHud           (const gcs::protocol::msg::VfrHud& v);
    void handleBatteryStatus    (const gcs::protocol::msg::BatteryStatus& b);
    void handleStatustext       (const gcs::protocol::msg::Statustext& t);

signals:
    void eventAppended(const QString& message);

    // Phase 6: surface link-state transitions and STATUSTEXT messages so
    // main.cpp can route them into the EventLogger without making Vehicle
    // depend on the logging module.
    void linkStateChanged(gcs::vehicle::LinkStatus oldStatus,
                          gcs::vehicle::LinkStatus newStatus,
                          qint64 ageMs);
    void statustextReceived(int severity, const QString& text, bool isPrearm);
    void heartbeatReceivedFirstTime();

private slots:
    void onStaleCheckTick();

private:
    int m_systemId;
    int m_componentId;
    gcs::firmware::FirmwarePlugin* m_firmware; // not owned
    gcs::mission::MissionManager*  m_mission  = nullptr;
    std::unique_ptr<VehicleStateStore> m_stateStore;
    QStringList m_events;
    QTimer m_staleCheck;
    static constexpr int kMaxEvents      = 200;
    static constexpr int kStaleThreshMs  = 3000;
    static constexpr int kLostThreshMs   = 10000;
};

} // namespace gcs::vehicle
