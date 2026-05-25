#pragma once

#include "LogEvent.h"

namespace gcs::logging {

// Sinks consume LogEvents. Implementations:
//   - MemoryLogSink: ring buffer for UI / tests
//   - FileLogSink:   appends one JSON-per-line to disk
//
// Sinks are called from the EventLogger on whichever thread emitted the
// event. Phase 6 runs single-threaded (Qt event loop / main thread), so
// implementations need not be thread-safe.
class LogSink
{
public:
    virtual ~LogSink() = default;
    virtual void write(const LogEvent& e) = 0;
};

} // namespace gcs::logging
