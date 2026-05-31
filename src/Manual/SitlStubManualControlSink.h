#pragma once

#include <QObject>
#include <QString>

#include "IManualControlSink.h"

namespace gcs::vehicle { class Vehicle; }

namespace gcs::manual {

// Logged-only manual-control sink for unsupported development transports.
//
// SAFETY: This sink deliberately does NOT pack or transmit MAVLink
// MANUAL_CONTROL. UDP/TCP SITL uses MavlinkManualControlSink; serial hardware
// gets no manual-control sink at all.
class SitlStubManualControlSink : public QObject, public IManualControlSink
{
    Q_OBJECT
public:
    explicit SitlStubManualControlSink(gcs::vehicle::Vehicle* vehicle,
                                       QObject* parent = nullptr);

    void onManualControlSample(int16_t x, int16_t y, int16_t z, int16_t r,
                               uint16_t buttons) override;
    QString sinkName() const override { return QStringLiteral("SITL-stub"); }
    bool    isSimulated() const override { return false; }

    int sampleCount() const { return m_sampleCount; }

private:
    gcs::vehicle::Vehicle* m_vehicle = nullptr;
    int m_sampleCount = 0;
    int m_lastLoggedAtMs = 0;
};

} // namespace gcs::manual
