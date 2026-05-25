#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Mission/MissionPlan.h"
#include "Mission/MissionUploader.h"
#include "Simulation/MockMissionLink.h"

using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::mission::MissionUploader;
using gcs::simulation::MockMissionLink;

namespace {
MissionPlan makePlan(int n)
{
    MissionPlan p;
    for (int i = 0; i < n; ++i) {
        MissionItem it;
        it.seq         = i;
        it.command     = gcs::mission::cmd::NavWaypoint;
        it.frame       = gcs::mission::frame::GlobalRelativeAltInt;
        it.latitudeDeg = 21.02 + i * 0.0001;
        it.longitudeDeg = 105.80 + i * 0.0001;
        it.altitudeM   = 50.0;
        p.items.append(it);
    }
    return p;
}

bool waitForCompletion(QSignalSpy& spy, int timeoutMs = 2000)
{
    return spy.wait(timeoutMs);
}
} // namespace

class TestMissionUploader : public QObject
{
    Q_OBJECT
private slots:
    void successWithThreeWaypoints();
    void rejectsConcurrentUpload();
    void wrongSeqAborts();
    void timeoutAfterRetries();
    void retryThenSucceed();
    void ackErrorFailsClean();
};

void TestMissionUploader::successWithThreeWaypoints()
{
    MockMissionLink link;
    MissionUploader up(&link);
    up.setTimeoutMs(200);

    QSignalSpy done(&up, &MissionUploader::completed);
    QSignalSpy progress(&up, &MissionUploader::progress);

    const auto plan = makePlan(3);
    QVERIFY(up.start(plan));
    QVERIFY(waitForCompletion(done));
    QCOMPARE(done.count(), 1);
    QCOMPARE(done.first().at(0).toBool(), true);
    QCOMPARE(link.lastReceivedCount(), 3);
    QCOMPARE(link.receivedItems().size(), 3);
    QCOMPARE(link.uploadAckResultSent(), int(MockMissionLink::Accepted));
}

void TestMissionUploader::rejectsConcurrentUpload()
{
    MockMissionLink link;
    MissionUploader up(&link);
    up.setTimeoutMs(200);

    const auto plan = makePlan(3);
    QVERIFY(up.start(plan));
    QVERIFY(!up.start(plan)); // second start while first is in flight
}

void TestMissionUploader::wrongSeqAborts()
{
    MockMissionLink link;
    link.setFaultMode(MockMissionLink::FaultMode::WrongRequestSeq);
    MissionUploader up(&link);
    up.setTimeoutMs(200);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(makePlan(3)));
    QVERIFY(waitForCompletion(done));
    QCOMPARE(done.first().at(0).toBool(), false);
    QVERIFY(done.first().at(1).toString().contains("expected"));
}

void TestMissionUploader::timeoutAfterRetries()
{
    MockMissionLink link;
    link.setFaultMode(MockMissionLink::FaultMode::DropAllResponses);
    MissionUploader up(&link);
    up.setTimeoutMs(30);
    up.setMaxRetries(2);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(makePlan(2)));
    QVERIFY(waitForCompletion(done, 1500));
    QCOMPARE(done.first().at(0).toBool(), false);
    QVERIFY(done.first().at(1).toString().contains("timed out"));
}

void TestMissionUploader::retryThenSucceed()
{
    MockMissionLink link;
    link.setFaultMode(MockMissionLink::FaultMode::DropOneItemThenSucceed);
    MissionUploader up(&link);
    up.setTimeoutMs(80);
    up.setMaxRetries(5);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(makePlan(2)));
    QVERIFY(waitForCompletion(done, 2000));
    QCOMPARE(done.first().at(0).toBool(), true);
}

void TestMissionUploader::ackErrorFailsClean()
{
    MockMissionLink link;
    link.setFaultMode(MockMissionLink::FaultMode::AckError);
    link.setAckResult(gcs::mission::IMissionLink::UnsupportedCommand);
    MissionUploader up(&link);
    up.setTimeoutMs(200);

    QSignalSpy done(&up, &MissionUploader::completed);
    QVERIFY(up.start(makePlan(2)));
    QVERIFY(waitForCompletion(done));
    QCOMPARE(done.first().at(0).toBool(), false);
    QVERIFY(done.first().at(1).toString().contains("ACK"));
}

QTEST_MAIN(TestMissionUploader)
#include "tst_mission_uploader.moc"
