# ADR-001: Use Qt 6 + QML + C++ for the desktop GCS

## Status
Accepted.

## Context
The lab needs a maintainable, cross-platform desktop GCS. The team has
prior experience reading QGroundControl's Qt/QML codebase but cannot afford
the maintenance burden of a fork.

## Decision
Build a new codebase in Qt 6 + QML + C++ with CMake. Use QGroundControl as
an *architectural reference* only.

## Consequences
- Cross-platform (Windows, Linux, macOS) out of the box.
- Familiar paradigm for anyone who has touched QGC.
- We must enforce a clean QML/C++ boundary ourselves: QML binds to
  ViewModel `Q_PROPERTY` values, never to MAVLink or backend internals.
