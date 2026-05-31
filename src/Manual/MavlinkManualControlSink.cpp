#include "MavlinkManualControlSink.h"

#include <QDateTime>
#include <algorithm>

#include "Comms/LinkInterface.h"
#include "Comms/LinkKind.h"
#include "Protocol/MavlinkFrame.h"
#include "Vehicle/Vehicle.h"

namespace gcs::manual {

namespace {

int16_t clampToI16(int value, int low, int high)
{
    return static_cast<int16_t>(std::clamp(value, low, high));
}

bool isSitlNetworkKind(gcs::comms::LinkKind kind)
{
    return kind == gcs::comms::LinkKind::Udp
        || kind == gcs::comms::LinkKind::Tcp;
}

} // namespace

MavlinkManualControlSink::MavlinkManualControlSink(
    int targetSystemId,
    gcs::comms::LinkInterface* outbound,
    int gcsSystemId,
    int gcsComponentId,
    gcs::vehicle::Vehicle* vehicle,
    QObject* parent)
    : QObject(parent),
      m_targetSysid(targetSystemId),
      m_gcsSysid(gcsSystemId),
      m_gcsCompid(gcsComponentId),
      m_outbound(outbound),
      m_vehicle(vehicle)
{
}

QString MavlinkManualControlSink::sinkName() const
{
    return QStringLiteral("MAVLink MANUAL_CONTROL");
}

void MavlinkManualControlSink::onManualControlSample(
    int16_t x, int16_t y, int16_t z, int16_t r, uint16_t buttons)
{
    ++m_sampleCount;

    if (!m_outbound || !m_outbound->isConnected()) {
        appendProblemThrottled(QStringLiteral(
            "[MANUAL] MAVLink sink has no connected outbound SITL link."));
        return;
    }
    if (!isSitlNetworkKind(m_outbound->kind())) {
        appendProblemThrottled(QStringLiteral(
            "[MANUAL] MAVLink sink refused non-SITL-network link."));
        return;
    }

    gcs::protocol::msg::ManualControl msg;
    msg.target  = static_cast<uint8_t>(std::clamp(m_targetSysid, 0, 255));
    msg.x       = clampToI16(x, -1000, 1000);
    msg.y       = clampToI16(y, -1000, 1000);
    // The current AxisMapper follows the PX4/QGC throttle convention:
    // MANUAL_CONTROL.z is sent as [0,1000] for multicopter throttle.
    msg.z       = clampToI16(z, 0, 1000);
    msg.r       = clampToI16(r, -1000, 1000);
    msg.buttons = buttons;

    const QByteArray payload = gcs::protocol::msg::encodeManualControl(msg);
    const QByteArray frame = gcs::protocol::buildV2Frame(
        m_gcsSysid, m_gcsCompid, gcs::protocol::msgid::ManualControl, payload);
    m_outbound->writeBytes(frame);

    if (m_sampleCount == 1 && m_vehicle) {
        m_vehicle->appendEvent(QStringLiteral(
            "[MANUAL] MAVLink MANUAL_CONTROL stream active (target sysid=%1).")
            .arg(m_targetSysid));
    }
}

void MavlinkManualControlSink::appendProblemThrottled(const QString& message)
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastProblemLogUtcMs < 1000) return;
    m_lastProblemLogUtcMs = now;
    if (m_vehicle) m_vehicle->appendEvent(message);
}

} // namespace gcs::manual
