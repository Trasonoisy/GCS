#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "MissionPlan.h"

namespace gcs::mission {

class IMissionLink;

// Drives the GCS side of the mission upload protocol:
//   GCS                                  Vehicle / Mock
//   ---  MISSION_COUNT(n)              -->
//                                       <-- MISSION_REQUEST_INT(0)
//   --- MISSION_ITEM_INT(items[0])    -->
//                                       <-- MISSION_REQUEST_INT(1)
//   ...
//   --- MISSION_ITEM_INT(items[n-1])  -->
//                                       <-- MISSION_ACK(Accepted)
//
// SAFETY: an upload is data-only. The vehicle does not arm, take off, or
// start the mission as a result of completion.
class MissionUploader : public QObject
{
    Q_OBJECT
public:
    enum class State {
        Idle,
        SendingCount,
        AwaitingRequest,
        AwaitingAck,
        Complete,
        Failed,
    };
    Q_ENUM(State)

    explicit MissionUploader(IMissionLink* link, QObject* parent = nullptr);

    State state() const { return m_state; }
    bool  isBusy() const;
    int   currentSeq() const { return m_currentSeq; }
    int   totalItems() const { return m_plan.items.size(); }

    void setTimeoutMs(int ms);
    void setMaxRetries(int n);

    // Returns false if an upload is already in progress (concurrent uploads
    // are rejected) or if the plan is empty.
    bool start(const MissionPlan& plan);
    void cancel(const QString& reason = QStringLiteral("cancelled"));

signals:
    void started(int totalItems);
    void progress(int sentItems, int totalItems);
    void completed(bool success, const QString& message);
    void stateChanged(State state);

private slots:
    void onRequestReceived(int seq);
    void onAckReceived(int result);
    void onTimeout();

private:
    void setState(State s);
    void resendCurrent();
    void fail(const QString& message);

    IMissionLink* m_link;
    MissionPlan   m_plan;
    State         m_state = State::Idle;

    int    m_currentSeq    = 0;
    int    m_retries       = 0;
    int    m_maxRetries    = 5;
    int    m_timeoutMs     = 1500;
    QTimer m_timer;
    enum class LastSent { Count, Item } m_lastSent = LastSent::Count;
};

} // namespace gcs::mission
