#pragma once

#include <QString>

namespace gcs::comms {

// Classifies how a LinkInterface delivers bytes. Used by:
//   - VehicleState (stamped at vehicle spawn so SafetyGate can branch by
//     transport kind without trusting autopilot identity alone)
//   - LinkViewModel (UI shows the active connection mode banner)
//
// Phase 7 introduces the "Serial" tier specifically so the safety layer can
// block manual control / mission upload to *real hardware* even when the
// autopilot reports `autopilot=PX4` (SITL and a real Pixhawk both do).
enum class LinkKind
{
    Unknown,
    Mock,     // MockLink driving MockVehicle
    Udp,      // UdpLink (PX4 / ArduPilot SITL via UDP)
    Tcp,      // TcpLink (reserved for ArduPilot SITL via 127.0.0.1:5760)
    Serial,   // SerialLink — real flight controller over USB or telemetry radio
    Replay,   // reserved for a future TlogReplayLink (NOT implemented)
};

inline QString linkKindName(LinkKind k)
{
    switch (k) {
        case LinkKind::Mock:    return QStringLiteral("Mock");
        case LinkKind::Udp:     return QStringLiteral("UDP");
        case LinkKind::Tcp:     return QStringLiteral("TCP");
        case LinkKind::Serial:  return QStringLiteral("Serial");
        case LinkKind::Replay:  return QStringLiteral("Replay");
        case LinkKind::Unknown: return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

} // namespace gcs::comms
