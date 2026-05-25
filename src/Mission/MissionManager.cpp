#include "MissionManager.h"

#include "IMissionLink.h"

namespace gcs::mission {

MissionManager::MissionManager(gcs::firmware::FirmwarePlugin* firmware,
                               IMissionLink* link,
                               QObject* parent)
    : QObject(parent),
      m_firmware(firmware),
      m_link(link),
      m_uploader(std::make_unique<MissionUploader>(link, this)),
      m_downloader(std::make_unique<MissionDownloader>(link, this))
{
    Q_ASSERT(link);

    connect(m_uploader.get(), &MissionUploader::started,
            this, &MissionManager::uploadStarted);
    connect(m_uploader.get(), &MissionUploader::progress,
            this, &MissionManager::uploadProgress);
    connect(m_uploader.get(), &MissionUploader::completed,
            this, &MissionManager::onUploadCompleted);

    connect(m_downloader.get(), &MissionDownloader::started,
            this, &MissionManager::downloadStarted);
    connect(m_downloader.get(), &MissionDownloader::progress,
            this, &MissionManager::downloadProgress);
    connect(m_downloader.get(), &MissionDownloader::completed,
            this, &MissionManager::onDownloadCompleted);
}

bool MissionManager::isBusy() const
{
    return m_uploader->isBusy() || m_downloader->isBusy();
}

bool MissionManager::startUpload(const MissionPlan& plan)
{
    if (isBusy()) {
        emit rejected(QStringLiteral(
            "Another mission transfer is already in progress"));
        return false;
    }
    m_currentMission = plan;
    return m_uploader->start(plan);
}

bool MissionManager::startDownload()
{
    if (isBusy()) {
        emit rejected(QStringLiteral(
            "Another mission transfer is already in progress"));
        return false;
    }
    return m_downloader->start();
}

void MissionManager::cancel(const QString& reason)
{
    if (m_uploader->isBusy())   m_uploader->cancel(reason);
    if (m_downloader->isBusy()) m_downloader->cancel(reason);
}

void MissionManager::onUploadCompleted(bool success, const QString& msg)
{
    emit uploadCompleted(success, msg);
}

void MissionManager::onDownloadCompleted(bool success, const QString& msg)
{
    if (success) {
        m_currentMission = m_downloader->result();
    }
    emit downloadCompleted(success, msg, m_downloader->result());
}

} // namespace gcs::mission
