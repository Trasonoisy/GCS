#include "MockMissionLink.h"

#include <QTimer>

namespace gcs::simulation {

using gcs::mission::MissionItem;

MockMissionLink::MockMissionLink(QObject* parent) : IMissionLink(parent) {}

// ---------- GCS-side outbound: we ARE the autopilot here ---------------

void MockMissionLink::sendMissionCount(int count)
{
    m_lastReceivedCount = count;
    m_expectedCount     = count;
    m_nextRequestSeq    = 0;
    m_receivedItems.clear();
    m_droppedItemCount  = 0;
    m_uploadAckSent     = -1;

    if (m_fault == FaultMode::DropAllResponses) return;
    postRequestForCurrentSeq();
}

void MockMissionLink::sendMissionItemInt(const MissionItem& item)
{
    if (m_fault == FaultMode::DropAllResponses) return;

    // Drop the very first item once, then resume — proves retry succeeds.
    if (m_fault == FaultMode::DropOneItemThenSucceed && m_droppedItemCount == 0) {
        ++m_droppedItemCount;
        return;
    }

    m_receivedItems.append(item);
    ++m_nextRequestSeq;

    if (m_nextRequestSeq >= m_expectedCount) {
        const int result = (m_fault == FaultMode::AckError)
                           ? m_ackResult
                           : static_cast<int>(Accepted);
        postUploadAck(result);
    } else {
        postRequestForCurrentSeq();
    }
}

void MockMissionLink::sendMissionAck(int result)
{
    // Used by the downloader to finalize. We don't model that side state.
    Q_UNUSED(result);
}

void MockMissionLink::sendMissionRequestList()
{
    if (m_fault == FaultMode::DropAllResponses) return;
    m_downloading = true;
    m_downloadSeq = 0;
    postCountToGcs(m_simulatedItems.size());
}

void MockMissionLink::sendMissionRequestInt(int seq)
{
    if (m_fault == FaultMode::DropAllResponses) return;
    if (seq < 0 || seq >= m_simulatedItems.size()) return;
    m_downloadSeq = seq;
    MissionItem it = m_simulatedItems.at(seq);
    it.seq = seq;
    postItemToGcs(it);
}

// ---------- async helpers: hop back to the event loop ------------------

void MockMissionLink::postRequestForCurrentSeq()
{
    int requestSeq = m_nextRequestSeq;
    if (m_fault == FaultMode::WrongRequestSeq) {
        // Skip ahead by one to trigger the uploader's out-of-order guard.
        requestSeq += 1;
    }
    QTimer::singleShot(0, this, [this, requestSeq] {
        emit missionRequestIntReceived(requestSeq);
    });
}

void MockMissionLink::postUploadAck(int result)
{
    m_uploadAckSent = result;
    QTimer::singleShot(0, this, [this, result] {
        emit missionAckReceived(result);
    });
}

void MockMissionLink::postCountToGcs(int count)
{
    QTimer::singleShot(0, this, [this, count] {
        emit missionCountReceived(count);
    });
}

void MockMissionLink::postItemToGcs(const MissionItem& it)
{
    QTimer::singleShot(0, this, [this, it] {
        emit missionItemIntReceived(it);
    });
}

} // namespace gcs::simulation
