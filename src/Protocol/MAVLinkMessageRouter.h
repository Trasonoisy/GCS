#pragma once

#include <QObject>
#include <functional>

#include "Comms/LinkKind.h"

namespace gcs::comms {
class LinkInterface;
}

namespace gcs::vehicle {
class MultiVehicleManager;
class Vehicle;
}

namespace gcs::protocol {

class MAVLinkProtocol;
namespace msg {
    struct Heartbeat;
    struct SysStatus;
    struct GpsRawInt;
    struct Attitude;
    struct GlobalPositionInt;
    struct VfrHud;
    struct BatteryStatus;
    struct Statustext;
}

// Routes decoded MAVLink messages from MAVLinkProtocol to the right Vehicle
// in MultiVehicleManager (by sysid/compid). On the first HEARTBEAT from an
// unknown identity, the router calls the configured vehicle factory to spawn
// one. Non-heartbeat messages from unknown vehicles are dropped (safe-by-
// default).
//
// SAFETY: This object only DELIVERS telemetry to existing Vehicles. It does
// not send anything outbound. Real command paths (arm/takeoff/RTL/etc.) wait
// for Phase 4+ behind SafetyGate.
class MAVLinkMessageRouter : public QObject
{
    Q_OBJECT
public:
    // Phase 5: the factory receives the MAV_TYPE byte so it can pick the
    // right ArduPilot subclass (Copter/Plane/Rover/Sub).
    // Phase 7: also receives the LinkKind that produced the spawning
    // heartbeat, so SafetyGate can branch on transport (Serial = hardware).
    // Phase 8: also receives the source LinkInterface* so the factory can
    // wire outbound MAVLink for SITL mission upload (writeBytes goes back
    // out the same link the heartbeat came in on). May be nullptr when the
    // router is fed from a test fixture without a real link.
    using VehicleFactory =
        std::function<gcs::vehicle::Vehicle*(int sysid, int compid,
                                             int autopilotType, int mavType,
                                             gcs::comms::LinkKind linkKind,
                                             gcs::comms::LinkInterface* sourceLink)>;

    explicit MAVLinkMessageRouter(QObject* parent = nullptr);
    ~MAVLinkMessageRouter() override = default;

    void setMultiVehicleManager(gcs::vehicle::MultiVehicleManager* m) { m_manager = m; }
    void setProtocol(MAVLinkProtocol* p);
    void setVehicleFactory(VehicleFactory f) { m_factory = std::move(f); }

    int routedMessageCount() const { return m_routedCount; }
    int droppedMessageCount() const { return m_droppedCount; }

signals:
    void vehicleSpawned(int sysid, int compid, int autopilotType);

private slots:
    void onHeartbeat        (int sysid, int compid, const msg::Heartbeat& hb);
    void onSysStatus        (int sysid, int compid, const msg::SysStatus& s);
    void onGpsRawInt        (int sysid, int compid, const msg::GpsRawInt& g);
    void onAttitude         (int sysid, int compid, const msg::Attitude& a);
    void onGlobalPositionInt(int sysid, int compid, const msg::GlobalPositionInt& p);
    void onVfrHud           (int sysid, int compid, const msg::VfrHud& v);
    void onBatteryStatus    (int sysid, int compid, const msg::BatteryStatus& b);
    void onStatustext       (int sysid, int compid, const msg::Statustext& t);

private:
    gcs::vehicle::Vehicle* lookup(int sysid, int compid) const;
    gcs::vehicle::Vehicle* lookupOrDrop(int sysid, int compid);

    gcs::vehicle::MultiVehicleManager* m_manager  = nullptr;
    MAVLinkProtocol*                   m_protocol = nullptr;
    VehicleFactory                     m_factory;
    int m_routedCount  = 0;
    int m_droppedCount = 0;
};

} // namespace gcs::protocol
