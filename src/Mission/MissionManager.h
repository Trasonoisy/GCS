#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "MissionPlan.h"
#include "MissionUploader.h"
#include "MissionDownloader.h"

namespace gcs::firmware { class FirmwarePlugin; }

namespace gcs::mission {

class IMissionLink;

// Owns the upload/download state machines for one Vehicle and rejects
// concurrent operations across both directions.
//
// SAFETY: This object is the only path through which a MissionPlan may be
// transmitted to or pulled from the vehicle. It never arms, takes off, or
// starts the mission as a side effect.
class MissionManager : public QObject
{
    Q_OBJECT
public:
    MissionManager(gcs::firmware::FirmwarePlugin* firmware,
                   IMissionLink* link,
                   QObject* parent = nullptr);

    gcs::firmware::FirmwarePlugin* firmware() const { return m_firmware; }
    IMissionLink* link() const { return m_link; }

    bool isBusy() const;

    MissionUploader*   uploader()   const { return m_uploader.get(); }
    MissionDownloader* downloader() const { return m_downloader.get(); }

    const MissionPlan& currentMission() const { return m_currentMission; }

    // Returns false (and emits rejected) if a transfer is already running.
    bool startUpload(const MissionPlan& plan);
    bool startDownload();
    void cancel(const QString& reason = QStringLiteral("cancelled"));

signals:
    void rejected(const QString& reason);

    void uploadStarted(int totalItems);
    void uploadProgress(int sentItems, int totalItems);
    void uploadCompleted(bool success, const QString& message);

    void downloadStarted();
    void downloadProgress(int receivedItems, int totalItems);
    void downloadCompleted(bool success, const QString& message,
                           const MissionPlan& downloadedPlan);

private slots:
    void onUploadCompleted(bool success, const QString& msg);
    void onDownloadCompleted(bool success, const QString& msg);

private:
    gcs::firmware::FirmwarePlugin*       m_firmware;
    IMissionLink*                        m_link;
    std::unique_ptr<MissionUploader>     m_uploader;
    std::unique_ptr<MissionDownloader>   m_downloader;
    MissionPlan                          m_currentMission;
};

} // namespace gcs::mission
