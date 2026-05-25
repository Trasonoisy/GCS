#pragma once

#include <QObject>

#include "JoystickState.h"

namespace gcs::manual {

// Abstract joystick backend. Concrete backends:
//   - MockJoystickBackend (Phase 4): driven by setters / UI sliders / tests
//   - SDLJoystickBackend  (TODO):    SDL2 polling — vendor SDL2 first
//
// SAFETY: Backends MUST NOT publish dangerous-button bindings. Buttons are
// exposed only as a bitmask in JoystickState; the rest of the system
// deliberately ignores them in Phase 4.
class JoystickBackend : public QObject
{
    Q_OBJECT
public:
    explicit JoystickBackend(QObject* parent = nullptr) : QObject(parent) {}
    ~JoystickBackend() override = default;

    virtual const JoystickState& state() const = 0;
    virtual bool isConnected() const = 0;

signals:
    // Emitted whenever the JoystickState was updated (any axis, button, or
    // connection change). The consumer should read state() to pull values.
    void stateChanged();
};

} // namespace gcs::manual
