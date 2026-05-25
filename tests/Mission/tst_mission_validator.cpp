#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"
#include "Mission/MissionPlan.h"
#include "Mission/MissionValidator.h"

using gcs::firmware::PX4FirmwarePlugin;
using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::mission::MissionValidator;
using gcs::mission::haversineKm;

class TestMissionValidator : public QObject
{
    Q_OBJECT
private slots:
    void emptyMissionIsError();
    void latitudeOutOfRange();
    void longitudeOutOfRange();
    void altitudeOutOfRange();
    void longLegProducesWarning();
    void firmwareUnsupportedCommandWarnsWhenListIsEmpty();
    void firmwareUnsupportedCommandErrorsWhenListIsPopulated();
    void happyPathProducesNoErrors();
    void haversineSanityCheck();
};

namespace {
MissionItem wpt(double lat, double lon, double alt)
{
    MissionItem it;
    it.command = gcs::mission::cmd::NavWaypoint;
    it.frame   = gcs::mission::frame::GlobalRelativeAltInt;
    it.latitudeDeg = lat;
    it.longitudeDeg = lon;
    it.altitudeM = alt;
    return it;
}
} // namespace

void TestMissionValidator::emptyMissionIsError()
{
    PX4FirmwarePlugin fw;
    MissionPlan plan;
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(!r.isValid());
    QVERIFY(r.errorMessages().first().contains("empty"));
}

void TestMissionValidator::latitudeOutOfRange()
{
    PX4FirmwarePlugin fw;
    MissionPlan plan;
    plan.items.append(wpt(91.0, 0.0, 50.0));
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(!r.isValid());
    QVERIFY(r.errorMessages().join(' ').contains("Latitude"));
}

void TestMissionValidator::longitudeOutOfRange()
{
    PX4FirmwarePlugin fw;
    MissionPlan plan;
    plan.items.append(wpt(0.0, -181.0, 50.0));
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(!r.isValid());
    QVERIFY(r.errorMessages().join(' ').contains("Longitude"));
}

void TestMissionValidator::altitudeOutOfRange()
{
    PX4FirmwarePlugin fw;
    MissionPlan plan;
    plan.items.append(wpt(0.0, 0.0, 999999.0));
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(!r.isValid());
    QVERIFY(r.errorMessages().join(' ').contains("Altitude"));
}

void TestMissionValidator::longLegProducesWarning()
{
    PX4FirmwarePlugin fw;
    MissionPlan plan;
    plan.items.append(wpt(10.0, 10.0, 50.0));
    plan.items.append(wpt(20.0, 20.0, 50.0)); // ~1500 km
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(r.isValid()); // no errors
    QVERIFY(r.hasWarnings());
    QVERIFY(r.warningMessages().join(' ').contains("km"));
}

void TestMissionValidator::firmwareUnsupportedCommandWarnsWhenListIsEmpty()
{
    // Phase 1/2 PX4FirmwarePlugin returns an empty supported-command list.
    PX4FirmwarePlugin fw;
    MissionPlan plan;
    plan.items.append(wpt(0.0, 0.0, 50.0));
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(r.isValid()); // not an error
    QVERIFY(r.hasWarnings());
    QVERIFY(r.warningMessages().join(' ').contains("Firmware command list"));
}

// A tiny firmware stub with a populated supported-command list, so we can
// exercise the error path of MissionValidator without changing PX4 defaults.
namespace {
class StubFirmware : public gcs::firmware::FirmwarePlugin
{
public:
    QString firmwareName() const override { return "stub"; }
    QString decodeFlightMode(uint8_t, uint32_t) const override { return ""; }
    bool encodeFlightMode(const QString&, uint8_t&, uint32_t&) const override { return false; }
    gcs::firmware::MissionFramePolicy missionFramePolicy() const override { return {}; }
    gcs::firmware::ManualControlPolicy manualControlPolicy() const override { return {}; }
    QList<int> supportedMissionCommands() const override
    { return { gcs::mission::cmd::NavWaypoint }; }
    QString prearmTextPrefix() const override { return ""; }
};
} // namespace

void TestMissionValidator::firmwareUnsupportedCommandErrorsWhenListIsPopulated()
{
    StubFirmware fw;
    MissionPlan plan;
    MissionItem it = wpt(0.0, 0.0, 50.0);
    it.command = gcs::mission::cmd::NavTakeoff; // not in stub's list
    plan.items.append(it);
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(!r.isValid());
    QVERIFY(r.errorMessages().join(' ').contains("not supported"));
}

void TestMissionValidator::happyPathProducesNoErrors()
{
    StubFirmware fw;
    MissionPlan plan;
    plan.items.append(wpt(21.028511, 105.804817, 50.0));
    plan.items.append(wpt(21.028611, 105.804917, 60.0));
    auto r = MissionValidator().validate(plan, &fw);
    QVERIFY(r.isValid());
}

void TestMissionValidator::haversineSanityCheck()
{
    // ~111 km per degree of latitude near the equator.
    const double km = haversineKm(0.0, 0.0, 1.0, 0.0);
    QVERIFY(km > 110.0 && km < 112.0);
}

QTEST_MAIN(TestMissionValidator)
#include "tst_mission_validator.moc"
