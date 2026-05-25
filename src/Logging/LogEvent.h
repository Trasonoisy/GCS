#pragma once

#include <QString>
#include <QVariantMap>
#include <QtGlobal>

namespace gcs::logging {

// Severity tier strings — kept as Qt-native strings so they serialise
// cleanly to JSONL without an enum-to-string indirection at every callsite.
namespace severity {
constexpr const char* Info  = "info";
constexpr const char* Warn  = "warn";
constexpr const char* Error = "error";
} // namespace severity

// Conventional category strings. These are NOT exhaustive — callers may
// invent new categories, but using a known one keeps log queries simple.
namespace category {
constexpr const char* App            = "App";
constexpr const char* Link           = "Link";
constexpr const char* Vehicle        = "Vehicle";
constexpr const char* Mission        = "Mission";
constexpr const char* Manual         = "Manual";
constexpr const char* Safety         = "Safety";
constexpr const char* OperatorAction = "OperatorAction";
constexpr const char* Telemetry      = "Telemetry";
} // namespace category

// Plain-data event passed to LogSinks. Lightweight so callers can hand it
// off by value without ceremony.
struct LogEvent
{
    qint64      timestampUtcMs = 0; // QDateTime::currentMSecsSinceEpoch()
    QString     category;
    QString     severity;
    QString     message;
    QVariantMap metadata;
};

} // namespace gcs::logging

Q_DECLARE_METATYPE(gcs::logging::LogEvent)
