#include "MockJoystickBackend.h"

#include <QDateTime>

#include "AxisMapper.h"

namespace gcs::manual {

MockJoystickBackend::MockJoystickBackend(QObject* parent)
    : JoystickBackend(parent)
{
}

void MockJoystickBackend::setConnected(bool c, const QString& name)
{
    if (m_state.connected == c && (!c || m_state.name == name)) return;
    m_state.connected = c;
    m_state.name      = c ? name : QString();
    if (!c) {
        // Re-centre everything on disconnect so a future reconnect does not
        // surprise the manager with stale stick values.
        m_rawPitch = m_rawRoll = m_rawThrottle = m_rawYaw = 0.0;
        m_state.buttons = 0;
    }
    recompute();
}

void MockJoystickBackend::setRawPitch(double v)    { m_rawPitch    = v; recompute(); }
void MockJoystickBackend::setRawRoll(double v)     { m_rawRoll     = v; recompute(); }
void MockJoystickBackend::setRawThrottle(double v) { m_rawThrottle = v; recompute(); }
void MockJoystickBackend::setRawYaw(double v)      { m_rawYaw      = v; recompute(); }

void MockJoystickBackend::setButtons(quint16 mask)
{
    if (m_state.buttons == mask) return;
    m_state.buttons = mask;
    m_state.lastUpdatedUtcMs = QDateTime::currentMSecsSinceEpoch();
    emit stateChanged();
}

void MockJoystickBackend::recompute()
{
    m_state.pitch    = axis::processAxis(m_rawPitch,    m_pitchCfg);
    m_state.roll     = axis::processAxis(m_rawRoll,     m_rollCfg);
    m_state.throttle = axis::processAxis(m_rawThrottle, m_throttleCfg);
    m_state.yaw      = axis::processAxis(m_rawYaw,      m_yawCfg);
    m_state.lastUpdatedUtcMs = QDateTime::currentMSecsSinceEpoch();
    emit stateChanged();
}

} // namespace gcs::manual
