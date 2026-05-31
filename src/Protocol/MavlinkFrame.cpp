#include "MavlinkFrame.h"

#include <cstring>

namespace gcs::protocol {

namespace {

template <typename T>
T readLE(const QByteArray& p, int offset, T dflt = T{})
{
    if (offset < 0 || offset + static_cast<int>(sizeof(T)) > p.size()) {
        return dflt;
    }
    T v;
    std::memcpy(&v, p.constData() + offset, sizeof(T));
    // MAVLink wire format is little-endian; x86 and ARM hosts are too.
    return v;
}

constexpr quint8 kStxV1 = 0xFE;
constexpr quint8 kStxV2 = 0xFD;

quint8 crcExtraForMessage(int msgid)
{
    // MAVLink common.xml CRC_EXTRA values for the messages this MVP builds.
    // PX4 validates these on outbound GCS frames; telemetry parsing remains
    // permissive because Phase 7 real-flight work will vendor c_library_v2.
    switch (msgid) {
        case msgid::Heartbeat:          return 50;
        case msgid::SysStatus:          return 124;
        case msgid::GpsRawInt:          return 24;
        case msgid::Attitude:           return 39;
        case msgid::GlobalPositionInt:  return 104;
        case msgid::ManualControl:      return 243;
        case msgid::VfrHud:             return 20;
        case msgid::BatteryStatus:      return 154;
        case msgid::Statustext:         return 83;
        case msgid::MissionRequestList: return 132;
        case msgid::MissionCount:       return 221;
        case msgid::MissionAck:         return 153;
        case msgid::MissionRequestInt:  return 196;
        case msgid::MissionItemInt:     return 38;
        default:                        return 0;
    }
}

void x25Accumulate(quint8 byte, quint16& crc)
{
    quint8 tmp = byte ^ quint8(crc & 0xFF);
    tmp ^= quint8(tmp << 4);
    crc = quint16((crc >> 8)
        ^ (quint16(tmp) << 8)
        ^ (quint16(tmp) << 3)
        ^ (quint16(tmp) >> 4));
}

quint16 mavlinkChecksum(const QByteArray& frameWithoutStx, quint8 crcExtra)
{
    quint16 crc = 0xFFFF;
    for (const char c : frameWithoutStx) {
        x25Accumulate(quint8(c), crc);
    }
    x25Accumulate(crcExtra, crc);
    return crc;
}

} // namespace

void MavlinkV2Parser::reset()
{
    m_buf.clear();
}

int MavlinkV2Parser::feed(const QByteArray& chunk, const Sink& sink)
{
    if (chunk.isEmpty() && m_buf.isEmpty()) return 0;
    m_buf.append(chunk);

    int emitted = 0;
    int i = 0;
    while (i < m_buf.size()) {
        const auto stx = quint8(m_buf[i]);
        if (stx != kStxV1 && stx != kStxV2) {
            ++i;
            continue;
        }
        const bool v2 = (stx == kStxV2);
        const int headerLen = v2 ? 10 : 6;
        if (m_buf.size() - i < headerLen) break; // wait for more bytes

        const int payloadLen = quint8(m_buf[i + 1]);
        int frameLen = headerLen + payloadLen + 2 /* CRC */;
        if (v2) {
            const auto incompatFlags = quint8(m_buf[i + 2]);
            if (incompatFlags & 0x01) frameLen += 13; // signature
        }
        if (m_buf.size() - i < frameLen) break;

        DecodedFrame f;
        f.v2 = v2;
        if (v2) {
            f.sysid  = quint8(m_buf[i + 5]);
            f.compid = quint8(m_buf[i + 6]);
            const auto a = quint8(m_buf[i + 7]);
            const auto b = quint8(m_buf[i + 8]);
            const auto c = quint8(m_buf[i + 9]);
            f.msgid = int(a | (b << 8) | (c << 16));
            f.payload = m_buf.mid(i + 10, payloadLen);
            ++m_v2Frames;
        } else {
            f.sysid  = quint8(m_buf[i + 3]);
            f.compid = quint8(m_buf[i + 4]);
            f.msgid  = quint8(m_buf[i + 5]);
            f.payload = m_buf.mid(i + 6, payloadLen);
            ++m_v1Frames;
        }
        sink(f);
        ++emitted;
        ++m_totalFrames;
        i += frameLen;
    }
    if (i > 0) m_buf.remove(0, i);
    return emitted;
}

QByteArray buildV2Frame(int sysid, int compid, int msgid, const QByteArray& payload)
{
    QByteArray out;
    out.reserve(10 + payload.size() + 2);
    out.append(char(kStxV2));
    out.append(char(payload.size())); // LEN
    out.append(char(0));               // incompat flags (no signature)
    out.append(char(0));               // compat flags
    out.append(char(0));               // seq
    out.append(char(quint8(sysid)));
    out.append(char(quint8(compid)));
    const auto m = quint32(msgid);
    out.append(char(m & 0xFF));
    out.append(char((m >> 8) & 0xFF));
    out.append(char((m >> 16) & 0xFF));
    out.append(payload);
    const quint16 crc = mavlinkChecksum(out.mid(1), crcExtraForMessage(msgid));
    out.append(char(crc & 0xFF));
    out.append(char((crc >> 8) & 0xFF));
    return out;
}

// ---------- Decoders ----------

namespace msg {

bool decodeHeartbeat(const QByteArray& p, Heartbeat& out)
{
    out.custom_mode     = readLE<quint32>(p, 0);
    out.type            = readLE<quint8>(p, 4);
    out.autopilot       = readLE<quint8>(p, 5);
    out.base_mode       = readLE<quint8>(p, 6);
    out.system_status   = readLE<quint8>(p, 7);
    out.mavlink_version = readLE<quint8>(p, 8);
    return true; // truncated payloads zero-extend per MAVLink v2
}

bool decodeSysStatus(const QByteArray& p, SysStatus& out)
{
    out.voltage_battery_mV = readLE<quint16>(p, 14);
    out.current_battery_cA = readLE<qint16>(p, 16);
    out.battery_remaining  = readLE<qint8>(p, 30, qint8(-1));
    return true;
}

bool decodeGpsRawInt(const QByteArray& p, GpsRawInt& out)
{
    out.fix_type           = readLE<quint8>(p, 28);
    out.satellites_visible = readLE<quint8>(p, 29);
    return true;
}

bool decodeAttitude(const QByteArray& p, Attitude& out)
{
    out.roll_rad   = readLE<float>(p, 4);
    out.pitch_rad  = readLE<float>(p, 8);
    out.yaw_rad    = readLE<float>(p, 12);
    out.rollspeed  = readLE<float>(p, 16);
    out.pitchspeed = readLE<float>(p, 20);
    out.yawspeed   = readLE<float>(p, 24);
    return true;
}

bool decodeGlobalPositionInt(const QByteArray& p, GlobalPositionInt& out)
{
    out.lat_e7          = readLE<qint32>(p, 4);
    out.lon_e7          = readLE<qint32>(p, 8);
    out.alt_mm          = readLE<qint32>(p, 12);
    out.relative_alt_mm = readLE<qint32>(p, 16);
    out.hdg_cdeg        = readLE<quint16>(p, 26, quint16(65535));
    return true;
}

bool decodeVfrHud(const QByteArray& p, VfrHud& out)
{
    out.airspeed_mps    = readLE<float>(p, 0);
    out.groundspeed_mps = readLE<float>(p, 4);
    out.alt_m           = readLE<float>(p, 8);
    out.climb_mps       = readLE<float>(p, 12);
    out.heading_deg     = readLE<qint16>(p, 16);
    out.throttle_pct    = readLE<quint16>(p, 18);
    return true;
}

bool decodeBatteryStatus(const QByteArray& p, BatteryStatus& out)
{
    out.current_consumed_mAh = readLE<qint32>(p, 4, qint32(-1));
    // battery_remaining lives at offset 35 in BATTERY_STATUS v2 — for our
    // immediate needs SYS_STATUS is enough, but expose it anyway.
    out.battery_remaining = readLE<qint8>(p, 35, qint8(-1));
    return true;
}

bool decodeStatustext(const QByteArray& p, Statustext& out)
{
    out.severity = readLE<quint8>(p, 0);
    const int max = qMin(p.size() - 1, 50);
    if (max <= 0) {
        out.text.clear();
    } else {
        QByteArray raw = p.mid(1, max);
        const int nul = raw.indexOf('\0');
        if (nul >= 0) raw.truncate(nul);
        out.text = QString::fromUtf8(raw);
    }
    return true;
}

bool decodeManualControl(const QByteArray& p, ManualControl& out)
{
    out.x       = readLE<qint16>(p, 0);
    out.y       = readLE<qint16>(p, 2);
    out.z       = readLE<qint16>(p, 4);
    out.r       = readLE<qint16>(p, 6);
    out.buttons = readLE<quint16>(p, 8);
    out.target  = readLE<quint8>(p, 10);
    return true;
}

// ---------- Phase 8: mission-protocol decoders ----------

bool decodeMissionCount(const QByteArray& p, MissionCount& out)
{
    out.count            = readLE<quint16>(p, 0);
    out.target_system    = readLE<quint8>(p, 2);
    out.target_component = readLE<quint8>(p, 3);
    return true;
}

bool decodeMissionRequestInt(const QByteArray& p, MissionRequestInt& out)
{
    out.seq              = readLE<quint16>(p, 0);
    out.target_system    = readLE<quint8>(p, 2);
    out.target_component = readLE<quint8>(p, 3);
    return true;
}

bool decodeMissionItemInt(const QByteArray& p, MissionItemInt& out)
{
    out.param1           = readLE<float>(p, 0);
    out.param2           = readLE<float>(p, 4);
    out.param3           = readLE<float>(p, 8);
    out.param4           = readLE<float>(p, 12);
    out.x                = readLE<qint32>(p, 16);
    out.y                = readLE<qint32>(p, 20);
    out.z                = readLE<float>(p, 24);
    out.seq              = readLE<quint16>(p, 28);
    out.command          = readLE<quint16>(p, 30);
    out.target_system    = readLE<quint8>(p, 32);
    out.target_component = readLE<quint8>(p, 33);
    out.frame            = readLE<quint8>(p, 34);
    out.current          = readLE<quint8>(p, 35);
    out.autocontinue     = readLE<quint8>(p, 36);
    out.mission_type     = readLE<quint8>(p, 37, quint8(0));
    return true;
}

bool decodeMissionAck(const QByteArray& p, MissionAck& out)
{
    out.target_system    = readLE<quint8>(p, 0);
    out.target_component = readLE<quint8>(p, 1);
    out.type             = readLE<quint8>(p, 2);
    return true;
}

bool decodeMissionRequestList(const QByteArray& p, MissionRequestList& out)
{
    out.target_system    = readLE<quint8>(p, 0);
    out.target_component = readLE<quint8>(p, 1);
    return true;
}

// ---------- Phase 8: mission-protocol encoders ----------
//
// Field order matches MAVLink wire format (largest-fixed-size first, then
// extension fields). Each encoder writes the entire payload without a frame
// header — wrap with buildV2Frame() for transmission.

namespace {
template <typename T>
void appendLE(QByteArray& out, T value)
{
    const int n = int(sizeof(T));
    char buf[sizeof(T)];
    std::memcpy(buf, &value, sizeof(T));
    out.append(buf, n);
}
} // namespace

QByteArray encodeManualControl(const ManualControl& in)
{
    QByteArray p; p.reserve(11);
    appendLE<qint16> (p, in.x);
    appendLE<qint16> (p, in.y);
    appendLE<qint16> (p, in.z);
    appendLE<qint16> (p, in.r);
    appendLE<quint16>(p, in.buttons);
    appendLE<quint8> (p, in.target);
    return p;
}

QByteArray encodeMissionCount(const MissionCount& in)
{
    QByteArray p; p.reserve(4);
    appendLE<quint16>(p, in.count);
    appendLE<quint8> (p, in.target_system);
    appendLE<quint8> (p, in.target_component);
    return p;
}

QByteArray encodeMissionRequestInt(const MissionRequestInt& in)
{
    QByteArray p; p.reserve(4);
    appendLE<quint16>(p, in.seq);
    appendLE<quint8> (p, in.target_system);
    appendLE<quint8> (p, in.target_component);
    return p;
}

QByteArray encodeMissionItemInt(const MissionItemInt& in)
{
    QByteArray p; p.reserve(38);
    appendLE<float>  (p, in.param1);
    appendLE<float>  (p, in.param2);
    appendLE<float>  (p, in.param3);
    appendLE<float>  (p, in.param4);
    appendLE<qint32> (p, in.x);
    appendLE<qint32> (p, in.y);
    appendLE<float>  (p, in.z);
    appendLE<quint16>(p, in.seq);
    appendLE<quint16>(p, in.command);
    appendLE<quint8> (p, in.target_system);
    appendLE<quint8> (p, in.target_component);
    appendLE<quint8> (p, in.frame);
    appendLE<quint8> (p, in.current);
    appendLE<quint8> (p, in.autocontinue);
    appendLE<quint8> (p, in.mission_type);
    return p;
}

QByteArray encodeMissionAck(const MissionAck& in)
{
    QByteArray p; p.reserve(3);
    appendLE<quint8>(p, in.target_system);
    appendLE<quint8>(p, in.target_component);
    appendLE<quint8>(p, in.type);
    return p;
}

QByteArray encodeMissionRequestList(const MissionRequestList& in)
{
    QByteArray p; p.reserve(2);
    appendLE<quint8>(p, in.target_system);
    appendLE<quint8>(p, in.target_component);
    return p;
}

} // namespace msg

} // namespace gcs::protocol
