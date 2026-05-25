#pragma once

#include "AxisConfig.h"
#include "JoystickBackend.h"

namespace gcs::manual {

// Test / UI-driven joystick. Raw setRawPitch/Roll/Throttle/Yaw input is run
// through AxisMapper (deadzone + expo + inversion) before being stored in the
// state struct. Connection is explicit (setConnected) — both tests and the
// UI must call it to come up Ready.
//
// SAFETY: Mock-only. Phase 4 does not link to any HID/SDL2 backend.
class MockJoystickBackend : public JoystickBackend
{
    Q_OBJECT
public:
    explicit MockJoystickBackend(QObject* parent = nullptr);

    const JoystickState& state() const override { return m_state; }
    bool isConnected() const override { return m_state.connected; }

    void setConnected(bool c, const QString& name = QStringLiteral("MockJoystick"));

    // Raw axis values in [-1, +1]; processed via AxisMapper.
    void setRawPitch   (double v);
    void setRawRoll    (double v);
    void setRawThrottle(double v);
    void setRawYaw     (double v);
    void setButtons    (quint16 mask);

    // Read back the current raw values (for UI binding).
    double rawPitch()    const { return m_rawPitch; }
    double rawRoll()     const { return m_rawRoll; }
    double rawThrottle() const { return m_rawThrottle; }
    double rawYaw()      const { return m_rawYaw; }

    AxisConfig& pitchConfig()    { return m_pitchCfg; }
    AxisConfig& rollConfig()     { return m_rollCfg; }
    AxisConfig& throttleConfig() { return m_throttleCfg; }
    AxisConfig& yawConfig()      { return m_yawCfg; }

private:
    void recompute();

    JoystickState m_state;
    double m_rawPitch    = 0.0;
    double m_rawRoll     = 0.0;
    double m_rawThrottle = 0.0;
    double m_rawYaw      = 0.0;
    AxisConfig m_pitchCfg;
    AxisConfig m_rollCfg;
    AxisConfig m_throttleCfg;
    AxisConfig m_yawCfg;
};

} // namespace gcs::manual
