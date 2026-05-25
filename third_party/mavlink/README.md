# third_party/mavlink

Placeholder for `mavlink/c_library_v2`.

Phase 3 will vendor the official headers here (either as a git submodule
or a manual copy) and wire them into `src/Protocol`. Until then, the
`Protocol` layer ships a stub so the architectural seam is reviewable.

See `docs/adr/ADR-002-use-mavlink-c-library-v2.md`.
