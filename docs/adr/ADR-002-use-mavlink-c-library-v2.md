# ADR-002: Target MAVLink c_library_v2 in the Protocol layer

## Status

Accepted. The current MVP uses a small local MAVLink 2 parser/encoder for
telemetry and mission-protocol messages. Before any real-flight command path is
enabled, the Protocol layer should move to MAVLink `c_library_v2` as the
production substrate.

## Context

MAVLink integration choices we evaluated:

- `mavlink/c_library_v2` - official generated headers, header-only.
- `MAVSDK` - high-level C++ wrapper, opinionated, harder to extend for
  research use cases.

## Decision

Use `c_library_v2` directly in the `Protocol` layer for the production
MAVLink path. Wrap it behind `MAVLinkProtocol` so the rest of the code does
not include MAVLink headers directly.

`MAVSDK` is allowed only for test scripts and optional experiment plugins,
never as the core protocol path.

## Consequences

- Full control over message handling, signing, and routing.
- Slightly more work than MAVSDK, but no opaque behavior.
- The MVP's local parser keeps the architecture testable, but it is not the
  final real-flight MAVLink implementation.

