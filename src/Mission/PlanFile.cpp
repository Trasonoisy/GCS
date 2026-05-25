#include "PlanFile.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <cmath>

#include "MissionItem.h"
#include "MissionPlan.h"

namespace gcs::mission {

namespace {

constexpr int kQgcMissionVersion = 2;
constexpr int kQgcFileVersion    = 1;

QJsonValue numOrNull(double v)
{
    if (std::isnan(v)) return QJsonValue();
    return QJsonValue(v);
}

double readNumOrNan(const QJsonValue& v)
{
    if (v.isNull() || !v.isDouble()) return qQNaN();
    return v.toDouble();
}

QJsonArray itemToParams(const MissionItem& it)
{
    // QGC params order: [hold, accept, passthrough, yaw, lat, lon, alt]
    QJsonArray a;
    a.append(it.holdTimeSec);
    a.append(it.acceptanceRadiusM);
    a.append(0.0);                          // pass-through radius (unused)
    a.append(numOrNull(it.yawDeg));
    a.append(it.latitudeDeg);
    a.append(it.longitudeDeg);
    a.append(it.altitudeM);
    return a;
}

void paramsToItem(const QJsonArray& a, MissionItem& it)
{
    auto pick = [&a](int i, double dflt) -> double {
        if (i >= a.size()) return dflt;
        const auto v = a.at(i);
        if (v.isNull() || !v.isDouble()) return dflt;
        return v.toDouble();
    };
    it.holdTimeSec       = pick(0, 0.0);
    it.acceptanceRadiusM = pick(1, 2.5);
    it.yawDeg            = readNumOrNan(a.size() > 3 ? a.at(3) : QJsonValue());
    it.latitudeDeg       = pick(4, 0.0);
    it.longitudeDeg      = pick(5, 0.0);
    it.altitudeM         = pick(6, 0.0);
}

} // namespace

QJsonObject PlanFile::toJson(const MissionPlan& plan)
{
    QJsonArray itemArr;
    int doJumpId = 1;
    for (const auto& it : plan.items) {
        QJsonObject o;
        o["type"]         = QStringLiteral("SimpleItem");
        o["autoContinue"] = it.autocontinue;
        o["command"]      = it.command;
        o["doJumpId"]     = doJumpId++;
        o["frame"]        = it.frame;
        o["params"]       = itemToParams(it);
        o["Altitude"]     = it.altitudeM;
        o["AltitudeMode"] = 1;             // 1 = relative-to-home
        itemArr.append(o);
    }

    QJsonArray homeArr;
    homeArr.append(std::isnan(plan.homeLatitudeDeg)  ? 0.0 : plan.homeLatitudeDeg);
    homeArr.append(std::isnan(plan.homeLongitudeDeg) ? 0.0 : plan.homeLongitudeDeg);
    homeArr.append(plan.homeAltitudeM);

    QJsonObject mission;
    mission["version"]              = kQgcMissionVersion;
    mission["firmwareType"]         = plan.firmwareType;
    mission["vehicleType"]          = plan.vehicleType;
    mission["cruiseSpeed"]          = plan.cruiseSpeedMps;
    mission["hoverSpeed"]           = plan.hoverSpeedMps;
    mission["defaultAltitude"]      = plan.defaultAltitudeM; // LabGCS extension
    mission["plannedHomePosition"]  = homeArr;
    mission["items"]                = itemArr;

    QJsonObject root;
    root["fileType"]      = QStringLiteral("Plan");
    root["groundStation"] = plan.groundStation;
    root["version"]       = kQgcFileVersion;
    root["mission"]       = mission;
    return root;
}

PlanFile::Result PlanFile::fromJson(MissionPlan& out, const QJsonObject& root)
{
    Result r;

    if (root.value("fileType").toString() != QStringLiteral("Plan")) {
        return { Status::WrongFileType, QStringLiteral("Missing or wrong fileType (expected \"Plan\")") };
    }

    out.fileType      = QStringLiteral("Plan");
    out.groundStation = root.value("groundStation").toString(QStringLiteral("LabGCS"));
    out.version       = root.value("version").toInt(kQgcFileVersion);

    const auto mission = root.value("mission").toObject();
    if (mission.isEmpty()) {
        return { Status::SchemaError, QStringLiteral("Missing \"mission\" object") };
    }

    out.firmwareType     = mission.value("firmwareType").toString(out.firmwareType);
    out.vehicleType      = mission.value("vehicleType").toString(out.vehicleType);
    out.cruiseSpeedMps   = mission.value("cruiseSpeed").toDouble(out.cruiseSpeedMps);
    out.hoverSpeedMps    = mission.value("hoverSpeed").toDouble(out.hoverSpeedMps);
    out.defaultAltitudeM = mission.value("defaultAltitude").toDouble(out.defaultAltitudeM);

    const auto home = mission.value("plannedHomePosition").toArray();
    if (home.size() >= 3) {
        out.homeLatitudeDeg  = home.at(0).toDouble();
        out.homeLongitudeDeg = home.at(1).toDouble();
        out.homeAltitudeM    = home.at(2).toDouble();
    }

    out.items.clear();
    const auto itemArr = mission.value("items").toArray();
    int seq = 0;
    for (const auto& v : itemArr) {
        const auto o = v.toObject();
        MissionItem it;
        it.seq          = seq++;
        it.command      = o.value("command").toInt(cmd::NavWaypoint);
        it.frame        = o.value("frame").toInt(frame::GlobalRelativeAltInt);
        it.autocontinue = o.value("autoContinue").toBool(true);

        const auto params = o.value("params").toArray();
        paramsToItem(params, it);

        // QGC sometimes carries Altitude separately — prefer it if present.
        if (o.contains("Altitude") && o.value("Altitude").isDouble()) {
            it.altitudeM = o.value("Altitude").toDouble();
        }
        out.items.append(it);
    }

    return r;
}

PlanFile::Result PlanFile::writeToFile(const MissionPlan& plan, const QString& path)
{
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return { Status::FilePermissionDenied,
                 QStringLiteral("Cannot open %1 for writing: %2").arg(path, f.errorString()) };
    }
    const auto doc = QJsonDocument(toJson(plan));
    f.write(doc.toJson(QJsonDocument::Indented));
    if (!f.commit()) {
        return { Status::FilePermissionDenied,
                 QStringLiteral("Failed to commit %1: %2").arg(path, f.errorString()) };
    }
    return {};
}

PlanFile::Result PlanFile::readFromFile(MissionPlan& out, const QString& path)
{
    QFile f(path);
    if (!f.exists()) {
        return { Status::FileNotFound, QStringLiteral("File not found: %1").arg(path) };
    }
    if (!f.open(QIODevice::ReadOnly)) {
        return { Status::FilePermissionDenied,
                 QStringLiteral("Cannot open %1: %2").arg(path, f.errorString()) };
    }
    QJsonParseError parseErr{};
    const auto doc = QJsonDocument::fromJson(f.readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        return { Status::InvalidJson,
                 QStringLiteral("Invalid JSON in %1: %2").arg(path, parseErr.errorString()) };
    }
    return fromJson(out, doc.object());
}

} // namespace gcs::mission
