#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Mission/MissionDownloader.h"
#include "Mission/MissionPlan.h"
#include "Simulation/MockMissionLink.h"

using gcs::mission::MissionDownloader;
using gcs::mission::MissionItem;
using gcs::simulation::MockMissionLink;

class TestMissionDownloader : public QObject
{
    Q_OBJECT
private slots:
    void downloadsThreeItems();
    void emptyMissionShortCircuits();
    void timeoutAfterRetries();
};

namespace {
QList<MissionItem> threeItems()
{
    QList<MissionItem> v;
    for (int i = 0; i < 3; ++i) {
        MissionItem it;
        it.seq         = i;
        it.command     = gcs::mission::cmd::NavWaypoint;
        it.frame       = gcs::mission::frame::GlobalRelativeAltInt;
        it.latitudeDeg = 10.0 + i;
        it.longitudeDeg = 20.0 + i;
        it.altitudeM    = 50.0 + i;
        v.append(it);
    }
    return v;
}
} // namespace

void TestMissionDownloader::downloadsThreeItems()
{
    MockMissionLink link;
    link.setSimulatedItems(threeItems());
    MissionDownloader dn(&link);
    dn.setTimeoutMs(200);

    QSignalSpy done(&dn, &MissionDownloader::completed);
    QVERIFY(dn.start());
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), true);
    QCOMPARE(dn.result().items.size(), 3);
    QCOMPARE(dn.result().items[2].latitudeDeg, 12.0);
}

void TestMissionDownloader::emptyMissionShortCircuits()
{
    MockMissionLink link;
    link.setSimulatedItems({});
    MissionDownloader dn(&link);
    dn.setTimeoutMs(200);

    QSignalSpy done(&dn, &MissionDownloader::completed);
    QVERIFY(dn.start());
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), true);
    QVERIFY(dn.result().items.isEmpty());
}

void TestMissionDownloader::timeoutAfterRetries()
{
    MockMissionLink link;
    link.setFaultMode(MockMissionLink::FaultMode::DropAllResponses);
    MissionDownloader dn(&link);
    dn.setTimeoutMs(30);
    dn.setMaxRetries(2);

    QSignalSpy done(&dn, &MissionDownloader::completed);
    QVERIFY(dn.start());
    QVERIFY(done.wait(2000));
    QCOMPARE(done.first().at(0).toBool(), false);
    QVERIFY(done.first().at(1).toString().contains("timed out"));
}

QTEST_MAIN(TestMissionDownloader)
#include "tst_mission_downloader.moc"
