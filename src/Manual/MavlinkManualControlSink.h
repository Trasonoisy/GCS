#pragma once

#include <QPointer>
#include <QObject>
#include <QString>

#include "IManualControlSink.h"

namespace gcs::comms { class LinkInterface; }
namespace gcs::vehicle { class Vehicle; }

namespace gcs::manual {

// Sends MAVLink MANUAL_CONTROL to a SITL autopilot over a network transport.
//
// SAFETY: this sink is still not a real-hardware control path. It refuses
// non-network links, and main.cpp only wires it for active UDP SITL vehicles.
// Serial/hardware links remain read-only and have no manual sink.
class MavlinkManualControlSink : public QObject, public IManualControlSink
{
    Q_OBJECT
public:
    MavlinkManualControlSink(int targetSystemId,
                             gcs::comms::LinkInterface* outbound,
                             int gcsSystemId = 255,
                             int gcsComponentId = 190,
                             gcs::vehicle::Vehicle* vehicle = nullptr,
                             QObject* parent = nullptr);

    void onManualControlSample(int16_t x, int16_t y, int16_t z, int16_t r,
                               uint16_t buttons) override;

    QString sinkName() const override;
    bool    isSimulated() const override { return false; }

    int sampleCount() const { return m_sampleCount; }

private:
    void appendProblemThrottled(const QString& message);

    int m_targetSysid;
    int m_gcsSysid;
    int m_gcsCompid;
    QPointer<gcs::comms::LinkInterface> m_outbound;
    QPointer<gcs::vehicle::Vehicle>     m_vehicle;
    int    m_sampleCount = 0;
    qint64 m_lastProblemLogUtcMs = 0;
};

} // namespace gcs::manual
