#include "MissionDownloader.h"

#include "IMissionLink.h"

namespace gcs::mission {

MissionDownloader::MissionDownloader(IMissionLink* link, QObject* parent)
    : QObject(parent), m_link(link)
{
    Q_ASSERT(m_link);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &MissionDownloader::onTimeout);
    connect(m_link, &IMissionLink::missionCountReceived,
            this, &MissionDownloader::onCountReceived);
    connect(m_link, &IMissionLink::missionItemIntReceived,
            this, &MissionDownloader::onItemReceived);
}

bool MissionDownloader::isBusy() const
{
    return m_state == State::AwaitingCount || m_state == State::AwaitingItem;
}

void MissionDownloader::setTimeoutMs(int ms) { if (ms > 0) m_timeoutMs = ms; }
void MissionDownloader::setMaxRetries(int n) { if (n > 0) m_maxRetries = n; }

bool MissionDownloader::start()
{
    if (isBusy()) return false; // concurrent downloads rejected
    m_plan = MissionPlan{};
    m_expectedCount = 0;
    m_currentSeq    = 0;
    m_retries       = 0;
    m_lastSent      = LastSent::RequestList;

    setState(State::AwaitingCount);
    emit started();

    m_link->sendMissionRequestList();
    m_timer.start(m_timeoutMs);
    return true;
}

void MissionDownloader::cancel(const QString& reason)
{
    if (!isBusy()) return;
    m_timer.stop();
    fail(reason);
}

void MissionDownloader::onCountReceived(int count)
{
    if (m_state != State::AwaitingCount) return;
    if (count < 0) {
        fail(QStringLiteral("Vehicle reported negative count %1").arg(count));
        return;
    }
    m_timer.stop();
    m_expectedCount = count;
    m_plan.items.clear();
    m_plan.items.reserve(count);
    emit progress(0, count);

    if (count == 0) {
        // Empty mission — acknowledge and finish.
        m_link->sendMissionAck(IMissionLink::Accepted);
        setState(State::Complete);
        emit completed(true, QStringLiteral("Downloaded empty mission"));
        setState(State::Idle);
        return;
    }

    m_currentSeq = 0;
    m_retries    = 0;
    setState(State::AwaitingItem);
    m_link->sendMissionRequestInt(0);
    m_lastSent = LastSent::RequestItem;
    m_timer.start(m_timeoutMs);
}

void MissionDownloader::onItemReceived(const MissionItem& item)
{
    if (m_state != State::AwaitingItem) return;
    if (item.seq != m_currentSeq) {
        // Out-of-order item: either a retransmit of a prior seq (ignore) or
        // a future seq (treat as protocol error).
        if (item.seq < m_currentSeq) return;
        fail(QStringLiteral("Vehicle sent seq %1, expected %2")
             .arg(item.seq).arg(m_currentSeq));
        return;
    }

    m_timer.stop();
    m_plan.items.append(item);
    ++m_currentSeq;
    m_retries = 0;
    emit progress(m_currentSeq, m_expectedCount);

    if (m_currentSeq >= m_expectedCount) {
        m_link->sendMissionAck(IMissionLink::Accepted);
        setState(State::Complete);
        emit completed(true, QStringLiteral("Downloaded %1 items").arg(m_expectedCount));
        setState(State::Idle);
        return;
    }

    m_link->sendMissionRequestInt(m_currentSeq);
    m_lastSent = LastSent::RequestItem;
    m_timer.start(m_timeoutMs);
}

void MissionDownloader::onTimeout()
{
    if (!isBusy()) return;
    if (m_retries >= m_maxRetries) {
        fail(QStringLiteral("Download timed out after %1 retries").arg(m_maxRetries));
        return;
    }
    ++m_retries;
    resendCurrentRequest();
    m_timer.start(m_timeoutMs);
}

void MissionDownloader::resendCurrentRequest()
{
    if (m_lastSent == LastSent::RequestList) {
        m_link->sendMissionRequestList();
    } else {
        m_link->sendMissionRequestInt(m_currentSeq);
    }
}

void MissionDownloader::fail(const QString& message)
{
    m_timer.stop();
    setState(State::Failed);
    emit completed(false, message);
    setState(State::Idle);
}

void MissionDownloader::setState(State s)
{
    if (m_state == s) return;
    m_state = s;
    emit stateChanged(s);
}

} // namespace gcs::mission
