#include "SitlStubManualControlSink.h"

#include <QDateTime>

#include "Vehicle/Vehicle.h"

namespace gcs::manual {

SitlStubManualControlSink::SitlStubManualControlSink(
    gcs::vehicle::Vehicle* vehicle, QObject* parent)
    : QObject(parent), m_vehicle(vehicle)
{
}

void SitlStubManualControlSink::onManualControlSample(
    int16_t x, int16_t y, int16_t z, int16_t r, uint16_t /*buttons*/)
{
    ++m_sampleCount;

    // Throttle log spam to ~1 Hz so the event log stays readable.
    const auto now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastLoggedAtMs < 1000) return;
    m_lastLoggedAtMs = static_cast<int>(now);

    if (!m_vehicle) return;
    m_vehicle->appendEvent(QStringLiteral(
        "[MANUAL stub] x=%1 y=%2 z=%3 r=%4 (no MAVLink sent)")
        .arg(x).arg(y).arg(z).arg(r));
}

} // namespace gcs::manual
