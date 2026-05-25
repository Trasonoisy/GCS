#pragma once

#include <QList>

#include "Mission/IMissionLink.h"
#include "Mission/MissionItem.h"

namespace gcs::simulation {

// Implements the *vehicle side* of the MAVLink mission protocol for
// hardware-free testing. The MissionUploader/Downloader on the GCS side
// believe they are talking to a real autopilot.
//
// Fault-injection hooks let tests drive the failure paths the brief calls
// out (timeout, wrong sequence, ACK rejection).
class MockMissionLink : public gcs::mission::IMissionLink
{
    Q_OBJECT
public:
    enum class FaultMode {
        None,
        DropAllResponses,      // never reply -> uploader/downloader times out
        WrongRequestSeq,       // for upload: send request for seq+1
        AckError,              // ACK with non-Accepted result
        DropOneItemThenSucceed,// drop the first item, then continue normally
    };
    Q_ENUM(FaultMode)

    explicit MockMissionLink(QObject* parent = nullptr);

    // IMissionLink (GCS-side outbound)
    void sendMissionCount(int count) override;
    void sendMissionItemInt(const gcs::mission::MissionItem& item) override;
    void sendMissionAck(int result) override;
    void sendMissionRequestList() override;
    void sendMissionRequestInt(int seq) override;

    // Test hooks
    void setFaultMode(FaultMode m)         { m_fault = m; }
    void setAckResult(int result)          { m_ackResult = result; }
    void setSimulatedItems(QList<gcs::mission::MissionItem> items)
    { m_simulatedItems = std::move(items); }

    int  lastReceivedCount() const         { return m_lastReceivedCount; }
    const QList<gcs::mission::MissionItem>& receivedItems() const
    { return m_receivedItems; }
    int  uploadAckResultSent() const       { return m_uploadAckSent; }

private:
    void postRequestForCurrentSeq();
    void postUploadAck(int result);
    void postCountToGcs(int count);
    void postItemToGcs(const gcs::mission::MissionItem& it);

    FaultMode m_fault     = FaultMode::None;
    int       m_ackResult = Accepted;

    // Upload-side (we receive from GCS)
    int m_expectedCount       = 0;
    int m_nextRequestSeq      = 0;
    int m_uploadAckSent       = -1;
    int m_lastReceivedCount   = -1;
    int m_droppedItemCount    = 0; // for DropOneItemThenSucceed
    QList<gcs::mission::MissionItem> m_receivedItems;

    // Download-side (GCS pulls from us)
    bool m_downloading = false;
    int  m_downloadSeq = 0;
    QList<gcs::mission::MissionItem> m_simulatedItems;
};

} // namespace gcs::simulation
