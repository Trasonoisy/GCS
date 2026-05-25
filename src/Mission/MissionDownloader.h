#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "MissionPlan.h"

namespace gcs::mission {

class IMissionLink;

// Drives the GCS side of the mission download protocol:
//   GCS                                   Vehicle / Mock
//   ---  MISSION_REQUEST_LIST          -->
//                                       <-- MISSION_COUNT(n)
//   ---  MISSION_REQUEST_INT(0)        -->
//                                       <-- MISSION_ITEM_INT(items[0])
//   ...
//   ---  MISSION_ACK(Accepted)         -->
class MissionDownloader : public QObject
{
    Q_OBJECT
public:
    enum class State {
        Idle,
        AwaitingCount,
        AwaitingItem,
        Complete,
        Failed,
    };
    Q_ENUM(State)

    explicit MissionDownloader(IMissionLink* link, QObject* parent = nullptr);

    State state() const { return m_state; }
    bool  isBusy() const;
    int   currentSeq() const { return m_currentSeq; }
    int   totalItems() const { return m_expectedCount; }
    const MissionPlan& result() const { return m_plan; }

    void setTimeoutMs(int ms);
    void setMaxRetries(int n);

    bool start();
    void cancel(const QString& reason = QStringLiteral("cancelled"));

signals:
    void started();
    void progress(int receivedItems, int totalItems);
    void completed(bool success, const QString& message);
    void stateChanged(State state);

private slots:
    void onCountReceived(int count);
    void onItemReceived(const MissionItem& item);
    void onTimeout();

private:
    void setState(State s);
    void resendCurrentRequest();
    void fail(const QString& message);

    IMissionLink* m_link;
    MissionPlan   m_plan;
    State         m_state = State::Idle;

    int    m_expectedCount = 0;
    int    m_currentSeq    = 0;
    int    m_retries       = 0;
    int    m_maxRetries    = 5;
    int    m_timeoutMs     = 1500;
    QTimer m_timer;
    enum class LastSent { RequestList, RequestItem } m_lastSent = LastSent::RequestList;
};

} // namespace gcs::mission
