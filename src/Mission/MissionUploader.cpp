#include "MissionUploader.h"

#include "IMissionLink.h"

namespace gcs::mission {

MissionUploader::MissionUploader(IMissionLink* link, QObject* parent)
    : QObject(parent), m_link(link)
{
    Q_ASSERT(m_link);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &MissionUploader::onTimeout);
    connect(m_link, &IMissionLink::missionRequestIntReceived,
            this, &MissionUploader::onRequestReceived);
    connect(m_link, &IMissionLink::missionAckReceived,
            this, &MissionUploader::onAckReceived);
}

bool MissionUploader::isBusy() const
{
    return m_state == State::SendingCount
        || m_state == State::AwaitingRequest
        || m_state == State::AwaitingAck;
}

void MissionUploader::setTimeoutMs(int ms)
{
    if (ms > 0) m_timeoutMs = ms;
}

void MissionUploader::setMaxRetries(int n)
{
    if (n > 0) m_maxRetries = n;
}

bool MissionUploader::start(const MissionPlan& plan)
{
    if (isBusy()) {
        // SAFETY: concurrent uploads are rejected — they could interleave
        // MISSION_ITEM_INT messages and corrupt the on-vehicle mission.
        return false;
    }
    if (plan.items.isEmpty()) {
        emit completed(false, QStringLiteral("Mission is empty"));
        return false;
    }

    m_plan       = plan;
    m_plan.renumberSequencesInPlace();
    m_currentSeq = 0;
    m_retries    = 0;
    m_lastSent   = LastSent::Count;

    setState(State::SendingCount);
    emit started(m_plan.items.size());
    emit progress(0, m_plan.items.size());

    m_link->sendMissionCount(m_plan.items.size());
    setState(State::AwaitingRequest);
    m_timer.start(m_timeoutMs);
    return true;
}

void MissionUploader::cancel(const QString& reason)
{
    if (!isBusy()) return;
    m_timer.stop();
    fail(reason);
}

void MissionUploader::onRequestReceived(int seq)
{
    if (m_state != State::AwaitingRequest) {
        // Late request after we've moved to AwaitingAck/Idle — ignore.
        return;
    }
    if (seq < 0 || seq >= m_plan.items.size()) {
        fail(QStringLiteral("Vehicle requested out-of-range seq %1 (expected 0..%2)")
             .arg(seq).arg(m_plan.items.size() - 1));
        return;
    }
    if (seq != m_currentSeq && seq != m_currentSeq - 1) {
        // Allow re-request of the previous seq (MAVLink retransmit). Anything
        // else is an unexpected jump and we abort to avoid undefined state.
        fail(QStringLiteral("Vehicle requested seq %1 but uploader expected %2")
             .arg(seq).arg(m_currentSeq));
        return;
    }

    // Reset retry budget on a fresh forward request.
    if (seq == m_currentSeq) m_retries = 0;

    m_timer.stop();
    m_link->sendMissionItemInt(m_plan.items.at(seq));
    m_lastSent = LastSent::Item;

    if (seq == m_currentSeq) {
        ++m_currentSeq;
        emit progress(m_currentSeq, m_plan.items.size());
    }

    if (m_currentSeq >= m_plan.items.size()) {
        setState(State::AwaitingAck);
    }
    m_timer.start(m_timeoutMs);
}

void MissionUploader::onAckReceived(int result)
{
    if (!isBusy()) return;
    m_timer.stop();

    if (m_state != State::AwaitingAck) {
        fail(QStringLiteral("Vehicle sent ACK (%1) before all items were uploaded")
             .arg(result));
        return;
    }

    if (result == IMissionLink::Accepted) {
        setState(State::Complete);
        emit completed(true, QStringLiteral("Upload accepted (%1 items)")
                       .arg(m_plan.items.size()));
        setState(State::Idle);
    } else {
        fail(QStringLiteral("Vehicle rejected mission with ACK result %1").arg(result));
    }
}

void MissionUploader::onTimeout()
{
    if (!isBusy()) return;

    if (m_retries >= m_maxRetries) {
        fail(QStringLiteral("Upload timed out after %1 retries").arg(m_maxRetries));
        return;
    }
    ++m_retries;
    resendCurrent();
    m_timer.start(m_timeoutMs);
}

void MissionUploader::resendCurrent()
{
    if (m_lastSent == LastSent::Count) {
        m_link->sendMissionCount(m_plan.items.size());
    } else {
        const int seq = qMax(0, m_currentSeq - 1);
        if (seq < m_plan.items.size()) {
            m_link->sendMissionItemInt(m_plan.items.at(seq));
        }
    }
}

void MissionUploader::fail(const QString& message)
{
    m_timer.stop();
    setState(State::Failed);
    emit completed(false, message);
    setState(State::Idle);
}

void MissionUploader::setState(State s)
{
    if (m_state == s) return;
    m_state = s;
    emit stateChanged(s);
}

} // namespace gcs::mission
