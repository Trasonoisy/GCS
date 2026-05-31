#pragma once

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <functional>

namespace gcs::protocol {

// MAVLink message IDs we consume. Names match the MAVLink common dialect so a
// future c_library_v2 swap is a renaming exercise rather than a rewiring
// exercise.
namespace msgid {
constexpr int Heartbeat          = 0;
constexpr int SysStatus          = 1;
constexpr int GpsRawInt          = 24;
constexpr int Attitude           = 30;
constexpr int GlobalPositionInt  = 33;
constexpr int VfrHud             = 74;
constexpr int BatteryStatus      = 147;
constexpr int Statustext         = 253;
// Phase 8: mission protocol messages used for SITL mission upload/download.
constexpr int MissionRequestList = 43;
constexpr int MissionCount       = 44;
constexpr int MissionAck         = 47;
constexpr int MissionRequestInt  = 51;
constexpr int MissionItemInt     = 73;
} // namespace msgid

// Raw decoded frame — payload bytes plus identity. Higher layers decode the
// payload per msgid using the helpers below.
//
// NOTE on CRC: Phase 3 telemetry over localhost UDP from SITL is reliable;
// we do not validate the X.25 CRC because doing so requires the per-msgid
// "extra CRC" byte table that ships with c_library_v2. This is documented in
// docs/safety.md — real-flight (Phase 7) must integrate c_library_v2 and
// validate.
struct DecodedFrame
{
    int        sysid   = 0;
    int        compid  = 0;
    int        msgid   = 0;
    bool       v2      = true;
    QByteArray payload;
};

// Frame-level parser. Accumulates byte chunks and emits complete frames via
// the sink callback. Survives garbage bytes by resyncing on the next STX.
class MavlinkV2Parser
{
public:
    using Sink = std::function<void(const DecodedFrame&)>;

    // Feed bytes; returns the number of complete frames produced.
    int feed(const QByteArray& bytes, const Sink& sink);
    void reset();

    // Diagnostics
    int totalFrames() const { return m_totalFrames; }
    int v1Frames()    const { return m_v1Frames; }
    int v2Frames()    const { return m_v2Frames; }

private:
    QByteArray m_buf;
    int m_totalFrames = 0;
    int m_v1Frames    = 0;
    int m_v2Frames    = 0;
};

// ---------- Typed payload structs and decoders ----------

namespace msg {

struct Heartbeat {
    uint32_t custom_mode    = 0;
    uint8_t  type           = 0;
    uint8_t  autopilot      = 0;
    uint8_t  base_mode      = 0;
    uint8_t  system_status  = 0;
    uint8_t  mavlink_version = 0;
};

struct SysStatus {
    uint16_t voltage_battery_mV = 0;
    int16_t  current_battery_cA = 0;
    int8_t   battery_remaining  = -1; // -1 = not estimated
};

struct GpsRawInt {
    uint8_t  fix_type            = 0;
    uint8_t  satellites_visible  = 0;
};

struct Attitude {
    float roll_rad      = 0.0f;
    float pitch_rad     = 0.0f;
    float yaw_rad       = 0.0f;
    float rollspeed     = 0.0f;
    float pitchspeed    = 0.0f;
    float yawspeed      = 0.0f;
};

struct GlobalPositionInt {
    int32_t  lat_e7         = 0;       // deg * 1e7
    int32_t  lon_e7         = 0;       // deg * 1e7
    int32_t  alt_mm         = 0;       // AMSL
    int32_t  relative_alt_mm = 0;
    uint16_t hdg_cdeg       = 65535;   // UINT16_MAX = unknown
};

struct VfrHud {
    float airspeed_mps    = 0.0f;
    float groundspeed_mps = 0.0f;
    float alt_m           = 0.0f;
    float climb_mps       = 0.0f;
    int16_t  heading_deg  = 0;
    uint16_t throttle_pct = 0;
};

struct BatteryStatus {
    int32_t current_consumed_mAh = -1;
    int8_t  battery_remaining     = -1;
};

struct Statustext {
    uint8_t severity = 0; // 0=emergency .. 7=debug
    QString text;
};

// Phase 8: mission-protocol messages. We deliberately model only the v1
// (non-extension) fields here — mission_type / opaque_id are reserved values
// in the lab build (always 0 = MISSION_TYPE_MISSION).
struct MissionCount {
    uint16_t count            = 0;
    uint8_t  target_system    = 0;
    uint8_t  target_component = 0;
};

struct MissionRequestInt {
    uint16_t seq              = 0;
    uint8_t  target_system    = 0;
    uint8_t  target_component = 0;
};

struct MissionItemInt {
    float    param1   = 0.0f;
    float    param2   = 0.0f;
    float    param3   = 0.0f;
    float    param4   = 0.0f;
    int32_t  x        = 0;       // lat * 1e7
    int32_t  y        = 0;       // lon * 1e7
    float    z        = 0.0f;    // altitude (frame-dependent)
    uint16_t seq      = 0;
    uint16_t command  = 0;
    uint8_t  target_system    = 0;
    uint8_t  target_component = 0;
    uint8_t  frame           = 0;
    uint8_t  current         = 0;
    uint8_t  autocontinue    = 0;
    uint8_t  mission_type    = 0; // extension; always 0 for our purposes
};

struct MissionAck {
    uint8_t target_system    = 0;
    uint8_t target_component = 0;
    uint8_t type             = 0; // MAV_MISSION_RESULT (Accepted = 0, Error = 1, …)
};

struct MissionRequestList {
    uint8_t target_system    = 0;
    uint8_t target_component = 0;
};

bool decodeHeartbeat        (const QByteArray& payload, Heartbeat& out);
bool decodeSysStatus        (const QByteArray& payload, SysStatus& out);
bool decodeGpsRawInt        (const QByteArray& payload, GpsRawInt& out);
bool decodeAttitude         (const QByteArray& payload, Attitude& out);
bool decodeGlobalPositionInt(const QByteArray& payload, GlobalPositionInt& out);
bool decodeVfrHud           (const QByteArray& payload, VfrHud& out);
bool decodeBatteryStatus    (const QByteArray& payload, BatteryStatus& out);
bool decodeStatustext       (const QByteArray& payload, Statustext& out);

// Phase 8 decoders.
bool decodeMissionCount       (const QByteArray& payload, MissionCount& out);
bool decodeMissionRequestInt  (const QByteArray& payload, MissionRequestInt& out);
bool decodeMissionItemInt     (const QByteArray& payload, MissionItemInt& out);
bool decodeMissionAck         (const QByteArray& payload, MissionAck& out);
bool decodeMissionRequestList (const QByteArray& payload, MissionRequestList& out);

// Phase 8 payload encoders. Each returns the wire-format payload bytes for
// the named message in MAVLink common-dialect field order. Wrap the result
// with buildV2Frame() to obtain a complete frame.
QByteArray encodeMissionCount       (const MissionCount& in);
QByteArray encodeMissionRequestInt  (const MissionRequestInt& in);
QByteArray encodeMissionItemInt     (const MissionItemInt& in);
QByteArray encodeMissionAck         (const MissionAck& in);
QByteArray encodeMissionRequestList (const MissionRequestList& in);

} // namespace msg

// ---------- Frame builder ----------

// Build a MAVLink v2 frame with no signature and the common.xml CRC_EXTRA
// values for the message IDs used by this MVP. The parser still does not
// validate inbound CRCs; real-flight (post-Phase-7) must vendor c_library_v2
// and validate all inbound/outbound dialect details.
QByteArray buildV2Frame(int sysid, int compid, int msgid, const QByteArray& payload);

} // namespace gcs::protocol
