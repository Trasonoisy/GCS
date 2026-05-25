#include <QTemporaryDir>
#include <QtTest/QtTest>

#include "Mission/MissionItem.h"
#include "Mission/MissionPlan.h"
#include "Mission/PlanFile.h"

using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::mission::PlanFile;

class TestPlanFile : public QObject
{
    Q_OBJECT
private slots:
    void jsonRoundTripPreservesItems();
    void fileRoundTrip();
    void rejectsNonPlanFileType();
    void rejectsMissingMissionObject();
};

namespace {
MissionPlan threeWaypointPlan()
{
    MissionPlan p;
    p.firmwareType  = "PX4";
    p.vehicleType   = "quadrotor";
    p.cruiseSpeedMps = 18.0;
    p.hoverSpeedMps  = 4.0;
    p.defaultAltitudeM = 75.0;
    p.homeLatitudeDeg  = 21.0285;
    p.homeLongitudeDeg = 105.8048;
    p.homeAltitudeM    = 0.0;
    for (int i = 0; i < 3; ++i) {
        MissionItem it;
        it.seq         = i;
        it.command     = gcs::mission::cmd::NavWaypoint;
        it.frame       = gcs::mission::frame::GlobalRelativeAltInt;
        it.latitudeDeg = 21.02 + i * 0.001;
        it.longitudeDeg = 105.80 + i * 0.001;
        it.altitudeM    = 50.0 + i;
        it.holdTimeSec  = 2.0;
        it.acceptanceRadiusM = 3.0;
        it.yawDeg = (i == 1) ? qQNaN() : (90.0 + i);
        p.items.append(it);
    }
    return p;
}
} // namespace

void TestPlanFile::jsonRoundTripPreservesItems()
{
    const auto in = threeWaypointPlan();
    const auto root = PlanFile::toJson(in);

    MissionPlan out;
    auto r = PlanFile::fromJson(out, root);
    QVERIFY(r.ok());

    QCOMPARE(out.firmwareType, in.firmwareType);
    QCOMPARE(out.vehicleType,  in.vehicleType);
    QCOMPARE(out.cruiseSpeedMps, in.cruiseSpeedMps);
    QCOMPARE(out.hoverSpeedMps,  in.hoverSpeedMps);
    QCOMPARE(out.defaultAltitudeM, in.defaultAltitudeM);
    QCOMPARE(out.homeLatitudeDeg,  in.homeLatitudeDeg);
    QCOMPARE(out.items.size(), in.items.size());
    for (int i = 0; i < in.items.size(); ++i) {
        QCOMPARE(out.items[i], in.items[i]);
    }
}

void TestPlanFile::fileRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("mission.plan");

    const auto in = threeWaypointPlan();
    auto rWrite = PlanFile::writeToFile(in, path);
    QVERIFY(rWrite.ok());

    MissionPlan out;
    auto rRead = PlanFile::readFromFile(out, path);
    QVERIFY(rRead.ok());
    QCOMPARE(out.items.size(), in.items.size());
    QCOMPARE(out.items.first(), in.items.first());
}

void TestPlanFile::rejectsNonPlanFileType()
{
    QJsonObject root;
    root["fileType"] = "Garbage";
    MissionPlan out;
    auto r = PlanFile::fromJson(out, root);
    QVERIFY(!r.ok());
    QCOMPARE(r.status, PlanFile::Status::WrongFileType);
}

void TestPlanFile::rejectsMissingMissionObject()
{
    QJsonObject root;
    root["fileType"] = "Plan";
    // no "mission" key
    MissionPlan out;
    auto r = PlanFile::fromJson(out, root);
    QVERIFY(!r.ok());
    QCOMPARE(r.status, PlanFile::Status::SchemaError);
}

QTEST_MAIN(TestPlanFile)
#include "tst_plan_file.moc"
