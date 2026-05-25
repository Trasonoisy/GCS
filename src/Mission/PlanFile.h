#pragma once

#include <QJsonObject>
#include <QString>

namespace gcs::mission {

struct MissionPlan;

// QGC-compatible .plan JSON subset for save/load.
//
// We follow the QGroundControl `mission` schema: a top-level "fileType":"Plan"
// envelope wraps a "mission" object containing a "plannedHomePosition" array
// and an "items" array. Each item carries QGC fields (command, frame,
// autoContinue, params[7], Altitude, AltitudeMode). Geofence and rallyPoints
// are intentionally omitted — they belong to a later phase.
class PlanFile
{
public:
    enum class Status {
        Ok = 0,
        FileNotFound,
        FilePermissionDenied,
        InvalidJson,
        WrongFileType,
        SchemaError,
    };

    struct Result {
        Status  status = Status::Ok;
        QString message;
        bool ok() const { return status == Status::Ok; }
    };

    static Result writeToFile(const MissionPlan& plan, const QString& path);
    static Result readFromFile(MissionPlan& out, const QString& path);

    // Pure JSON helpers — useful for round-trip tests without touching disk.
    static QJsonObject toJson(const MissionPlan& plan);
    static Result      fromJson(MissionPlan& out, const QJsonObject& root);
};

} // namespace gcs::mission
