#include "MAVLinkMessageRouter.h"

#include "Comms/LinkInterface.h"
#include "MAVLinkProtocol.h"
#include "MavlinkFrame.h"
#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"

namespace gcs::protocol {

MAVLinkMessageRouter::MAVLinkMessageRouter(QObject* parent) : QObject(parent) {}

void MAVLinkMessageRouter::setProtocol(MAVLinkProtocol* p)
{
    if (m_protocol) disconnect(m_protocol, nullptr, this, nullptr);
    m_protocol = p;
    if (!p) return;

    connect(p, &MAVLinkProtocol::heartbeatReceived,
            this, &MAVLinkMessageRouter::onHeartbeat);
    connect(p, &MAVLinkProtocol::sysStatusReceived,
            this, &MAVLinkMessageRouter::onSysStatus);
    connect(p, &MAVLinkProtocol::gpsRawIntReceived,
            this, &MAVLinkMessageRouter::onGpsRawInt);
    connect(p, &MAVLinkProtocol::attitudeReceived,
            this, &MAVLinkMessageRouter::onAttitude);
    connect(p, &MAVLinkProtocol::globalPositionIntReceived,
            this, &MAVLinkMessageRouter::onGlobalPositionInt);
    connect(p, &MAVLinkProtocol::vfrHudReceived,
            this, &MAVLinkMessageRouter::onVfrHud);
    connect(p, &MAVLinkProtocol::batteryStatusReceived,
            this, &MAVLinkMessageRouter::onBatteryStatus);
    connect(p, &MAVLinkProtocol::statustextReceived,
            this, &MAVLinkMessageRouter::onStatustext);
}

gcs::vehicle::Vehicle* MAVLinkMessageRouter::lookup(int sysid, int compid) const
{
    if (!m_manager) return nullptr;
    return m_manager->findBySysCompId(sysid, compid);
}

gcs::vehicle::Vehicle* MAVLinkMessageRouter::lookupOrDrop(int sysid, int compid)
{
    auto* v = lookup(sysid, compid);
    if (!v) {
        ++m_droppedCount;
        // SAFETY: telemetry from an unknown vehicle is silently dropped.
        // Only a HEARTBEAT may register a new vehicle — see onHeartbeat.
    }
    return v;
}

void MAVLinkMessageRouter::onHeartbeat(int sysid, int compid, const msg::Heartbeat& hb)
{
    auto* v = lookup(sysid, compid);
    if (!v) {
        if (m_factory) {
            // Phase 7: peek the source link's kind so the factory can stamp
            // the new Vehicle (and SafetyGate can distinguish hardware vs
            // SITL even when the autopilot identity is the same).
            // Phase 8: also hand over the LinkInterface* itself so the
            // factory can wire outbound mission MAVLink back through it.
            gcs::comms::LinkKind lk = gcs::comms::LinkKind::Unknown;
            gcs::comms::LinkInterface* srcLink = nullptr;
            if (m_protocol) {
                srcLink = m_protocol->currentSourceLink();
                if (srcLink) lk = srcLink->kind();
            }
            v = m_factory(sysid, compid, int(hb.autopilot), int(hb.type),
                          lk, srcLink);
            if (v) emit vehicleSpawned(sysid, compid, int(hb.autopilot));
        }
        if (!v) {
            ++m_droppedCount;
            return;
        }
    }
    v->handleHeartbeat(hb);
    ++m_routedCount;
}

void MAVLinkMessageRouter::onSysStatus(int sysid, int compid, const msg::SysStatus& s)
{
    if (auto* v = lookupOrDrop(sysid, compid)) { v->handleSysStatus(s); ++m_routedCount; }
}

void MAVLinkMessageRouter::onGpsRawInt(int sysid, int compid, const msg::GpsRawInt& g)
{
    if (auto* v = lookupOrDrop(sysid, compid)) { v->handleGpsRawInt(g); ++m_routedCount; }
}

void MAVLinkMessageRouter::onAttitude(int sysid, int compid, const msg::Attitude& a)
{
    if (auto* v = lookupOrDrop(sysid, compid)) { v->handleAttitude(a); ++m_routedCount; }
}

void MAVLinkMessageRouter::onGlobalPositionInt(int sysid, int compid,
                                               const msg::GlobalPositionInt& p)
{
    if (auto* v = lookupOrDrop(sysid, compid)) { v->handleGlobalPositionInt(p); ++m_routedCount; }
}

void MAVLinkMessageRouter::onVfrHud(int sysid, int compid, const msg::VfrHud& v)
{
    if (auto* veh = lookupOrDrop(sysid, compid)) { veh->handleVfrHud(v); ++m_routedCount; }
}

void MAVLinkMessageRouter::onBatteryStatus(int sysid, int compid, const msg::BatteryStatus& b)
{
    if (auto* v = lookupOrDrop(sysid, compid)) { v->handleBatteryStatus(b); ++m_routedCount; }
}

void MAVLinkMessageRouter::onStatustext(int sysid, int compid, const msg::Statustext& t)
{
    if (auto* v = lookupOrDrop(sysid, compid)) { v->handleStatustext(t); ++m_routedCount; }
}

} // namespace gcs::protocol
