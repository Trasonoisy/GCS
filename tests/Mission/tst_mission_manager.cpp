#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"
#include "Mission/MissionManager.h"
#include "Mission/MissionPlan.h"
#include "Simulation/MockMissionLink.h"

using gcs::firmware::PX4FirmwarePlugin;
using gcs::mission::MissionItem;
using gcs::mission::MissionManager;
using gcs::mission::MissionPlan;
using gcs::simulation::MockMissionLink;

class TestMissionManager : public QObject
{
    Q_OBJECT
private slots:
    void uploadThenDownloadWorks();
    void rejectsConcurrentTransfers();
};

namespace {
MissionPlan makePlan(int n)
{
    MissionPlan p;
    for (int i = 0; i < n; ++i) {
        MissionItem it;
        it.seq         = i;
        it.command     = gcs::mission::cmd::NavWaypoint;
        it.frame       = gcs::mission::frame::GlobalRelativeAltInt;
        it.latitudeDeg = 21.0 + i * 0.001;
        it.longitudeDeg = 105.0 + i * 0.001;
        it.altitudeM   = 50.0;
        p.items.append(it);
    }
    return p;
}
} // namespace

void TestMissionManager::uploadThenDownloadWorks()
{
    PX4FirmwarePlugin fw;
    MockMissionLink   link;
    MissionManager    mgr(&fw, &link);
    mgr.uploader()->setTimeoutMs(200);
    mgr.downloader()->setTimeoutMs(200);

    QSignalSpy up(&mgr, &MissionManager::uploadCompleted);
    QVERIFY(mgr.startUpload(makePlan(3)));
    QVERIFY(up.wait(2000));
    QCOMPARE(up.first().at(0).toBool(), true);

    // Now feed the same items back into the simulated "vehicle storage" so
    // download has something to return, then download.
    link.setSimulatedItems(link.receivedItems());

    QSignalSpy dn(&mgr, &MissionManager::downloadCompleted);
    QVERIFY(mgr.startDownload());
    QVERIFY(dn.wait(2000));
    QCOMPARE(dn.first().at(0).toBool(), true);
}

void TestMissionManager::rejectsConcurrentTransfers()
{
    PX4FirmwarePlugin fw;
    MockMissionLink   link;
    link.setFaultMode(MockMissionLink::FaultMode::DropAllResponses);
    MissionManager mgr(&fw, &link);
    mgr.uploader()->setTimeoutMs(500);

    QSignalSpy rejected(&mgr, &MissionManager::rejected);

    QVERIFY(mgr.startUpload(makePlan(3)));    // first one is in flight
    QVERIFY(!mgr.startUpload(makePlan(3)));   // second upload rejected
    QVERIFY(!mgr.startDownload());            // also can't download mid-upload
    QCOMPARE(rejected.count(), 2);
}

QTEST_MAIN(TestMissionManager)
#include "tst_mission_manager.moc"
