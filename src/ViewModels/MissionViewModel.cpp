#include "MissionViewModel.h"

#include <QDateTime>
#include <QUrl>
#include <cmath>

#include "Firmware/FirmwarePlugin.h"
#include "Mission/MissionManager.h"
#include "Mission/MissionValidator.h"
#include "Mission/PlanFile.h"
#include "Safety/SafetyGate.h"
#include "Simulation/MockVehicle.h"
#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

namespace gcs::viewmodels {

using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::mission::MissionValidator;
using gcs::mission::ValidationResult;
using gcs::mission::PlanFile;

namespace {
QString normalizePath(const QString& pathOrUrl)
{
    const QUrl url(pathOrUrl);
    if (url.isLocalFile()) return url.toLocalFile();
    return pathOrUrl;
}

bool linkFreshForTransfer(const gcs::vehicle::VehicleState& s)
{
    if (s.linkStatus != gcs::vehicle::LinkStatus::Connected) return false;
    if (s.lastHeartbeatUtcMs <= 0) return false;
    const qint64 age = QDateTime::currentMSecsSinceEpoch() - s.lastHeartbeatUtcMs;
    return age >= 0 && age <= gcs::safety::SafetyGate::kFreshHeartbeatMs;
}
} // namespace

MissionViewModel::MissionViewModel(gcs::vehicle::MultiVehicleManager* manager,
                                   QObject* parent)
    : QObject(parent), m_manager(manager)
{
    Q_ASSERT(m_manager);
    connect(m_manager, &gcs::vehicle::MultiVehicleManager::activeVehicleChanged,
            this, &MissionViewModel::onActiveVehicleChanged);
    onActiveVehicleChanged(m_manager->activeVehicle());
}

void MissionViewModel::setMissionPreviewMockVehicle(
    gcs::simulation::MockVehicle* mockVehicle)
{
    if (m_previewMockVehicle == mockVehicle) return;
    if (m_previewMockVehicle) {
        disconnect(m_previewMockVehicle, nullptr, this, nullptr);
    }

    m_previewMockVehicle = mockVehicle;

    if (m_previewMockVehicle) {
        connect(m_previewMockVehicle,
                &gcs::simulation::MockVehicle::missionPreviewProgressChanged,
                this, &MissionViewModel::onMissionPreviewProgress);
        connect(m_previewMockVehicle,
                &gcs::simulation::MockVehicle::missionPreviewCompleted,
                this, &MissionViewModel::onMissionPreviewCompleted);
        connect(m_previewMockVehicle,
                &gcs::simulation::MockVehicle::missionPreviewStopped,
                this, &MissionViewModel::onMissionPreviewStopped);
    }

    emit missionPreviewChanged();
}

QVariantList MissionViewModel::items() const
{
    QVariantList out;
    out.reserve(m_plan.items.size());
    for (int i = 0; i < m_plan.items.size(); ++i) {
        out.append(itemToMap(m_plan.items.at(i), i));
    }
    return out;
}

QVariantMap MissionViewModel::itemToMap(const MissionItem& it, int index) const
{
    QVariantMap m;
    m["index"]             = index;
    m["seq"]               = it.seq;
    m["command"]           = it.command;
    m["commandName"]       = gcs::mission::commandName(it.command);
    m["frame"]             = it.frame;
    m["frameName"]         = gcs::mission::frameName(it.frame);
    m["latitudeDeg"]       = it.latitudeDeg;
    m["longitudeDeg"]      = it.longitudeDeg;
    m["altitudeM"]         = it.altitudeM;
    m["holdTimeSec"]       = it.holdTimeSec;
    m["acceptanceRadiusM"] = it.acceptanceRadiusM;
    m["yawDeg"]            = std::isnan(it.yawDeg) ? QVariant() : QVariant(it.yawDeg);
    m["autocontinue"]      = it.autocontinue;
    return m;
}

QVariantMap MissionViewModel::selectedItem() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_plan.items.size()) {
        return {};
    }
    return itemToMap(m_plan.items.at(m_selectedIndex), m_selectedIndex);
}

void MissionViewModel::setSelectedIndex(int i)
{
    if (i < -1 || i >= m_plan.items.size()) i = -1;
    if (m_selectedIndex == i) return;
    m_selectedIndex = i;
    emit selectedIndexChanged();
    emit selectedItemChanged();
}

void MissionViewModel::addWaypoint()
{
    MissionItem it;
    it.seq         = m_plan.items.size();
    it.command     = gcs::mission::cmd::NavWaypoint;
    it.frame       = gcs::mission::frame::GlobalRelativeAltInt;
    it.altitudeM   = m_plan.defaultAltitudeM;
    it.holdTimeSec = 0.0;
    it.acceptanceRadiusM = 2.5;
    it.yawDeg      = qQNaN();
    it.autocontinue = true;

    // Seed lat/lon from the previous waypoint if any. For the first item,
    // use the active vehicle's current fix when available so the operator
    // does not accidentally create a mission at 0/0.
    if (!m_plan.items.isEmpty()) {
        const auto& prev = m_plan.items.last();
        it.latitudeDeg  = prev.latitudeDeg;
        it.longitudeDeg = prev.longitudeDeg;
    } else if (m_activeVehicle) {
        const auto& s = m_activeVehicle->stateStore()->state();
        if (std::isfinite(s.latitudeDeg) && std::isfinite(s.longitudeDeg)
            && !(s.latitudeDeg == 0.0 && s.longitudeDeg == 0.0)) {
            it.latitudeDeg  = s.latitudeDeg;
            it.longitudeDeg = s.longitudeDeg;
        }
    }
    m_plan.items.append(it);
    markDirty();
    emit itemsChanged();
    emit missionPreviewChanged();
    setSelectedIndex(m_plan.items.size() - 1);
}

void MissionViewModel::deleteWaypoint(int index)
{
    if (index < 0 || index >= m_plan.items.size()) return;
    m_plan.items.removeAt(index);
    m_plan.renumberSequencesInPlace();
    markDirty();
    emit itemsChanged();
    emit missionPreviewChanged();
    if (m_selectedIndex >= m_plan.items.size()) {
        setSelectedIndex(m_plan.items.size() - 1);
    } else {
        emit selectedItemChanged();
    }
}

void MissionViewModel::moveWaypoint(int fromIndex, int toIndex)
{
    if (fromIndex == toIndex) return;
    if (fromIndex < 0 || fromIndex >= m_plan.items.size()) return;
    if (toIndex   < 0 || toIndex   >= m_plan.items.size()) return;
    m_plan.items.move(fromIndex, toIndex);
    m_plan.renumberSequencesInPlace();
    markDirty();
    emit itemsChanged();
    emit missionPreviewChanged();
    setSelectedIndex(toIndex);
}

void MissionViewModel::updateWaypointField(int index,
                                           const QString& field,
                                           const QVariant& value)
{
    if (index < 0 || index >= m_plan.items.size()) return;
    MissionItem& it = m_plan.items[index];

    if      (field == "command")           it.command           = value.toInt();
    else if (field == "frame")             it.frame             = value.toInt();
    else if (field == "latitudeDeg")       it.latitudeDeg       = value.toDouble();
    else if (field == "longitudeDeg")      it.longitudeDeg      = value.toDouble();
    else if (field == "altitudeM")         it.altitudeM         = value.toDouble();
    else if (field == "holdTimeSec")       it.holdTimeSec       = value.toDouble();
    else if (field == "acceptanceRadiusM") it.acceptanceRadiusM = value.toDouble();
    else if (field == "yawDeg")            it.yawDeg            = value.canConvert<double>() ? value.toDouble() : qQNaN();
    else if (field == "autocontinue")      it.autocontinue      = value.toBool();
    else return;

    markDirty();
    emit itemsChanged();
    emit missionPreviewChanged();
    if (index == m_selectedIndex) emit selectedItemChanged();
}

void MissionViewModel::clearMission()
{
    if (m_plan.items.isEmpty()) return;
    m_plan.items.clear();
    setSelectedIndex(-1);
    markDirty();
    emit itemsChanged();
    emit missionPreviewChanged();
}

void MissionViewModel::validateMission()
{
    MissionValidator v;
    auto* fw = activeFirmware();
    const ValidationResult r = v.validate(m_plan, fw);
    m_validationErrors   = r.errorMessages();
    m_validationWarnings = r.warningMessages();
    m_validationRun      = true;
    emit validationChanged();

    if (r.isValid()) {
        setStatus(QStringLiteral("Validation passed (%1 warning(s))")
                  .arg(m_validationWarnings.size()));
    } else {
        setStatus(QStringLiteral("Validation failed: %1 error(s)")
                  .arg(m_validationErrors.size()));
    }
}

bool MissionViewModel::saveToFile(const QString& path)
{
    const QString p = normalizePath(path);
    const auto r = PlanFile::writeToFile(m_plan, p);
    if (!r.ok()) {
        setStatus(QStringLiteral("Save failed: %1").arg(r.message));
        emit errorMessage(r.message);
        return false;
    }
    m_dirty = false;
    m_currentFilePath = p;
    emit dirtyChanged();
    emit currentFilePathChanged();
    setStatus(QStringLiteral("Saved to %1").arg(p));
    emit infoMessage(QStringLiteral("Saved %1").arg(p));
    return true;
}

bool MissionViewModel::loadFromFile(const QString& path)
{
    const QString p = normalizePath(path);
    MissionPlan loaded;
    const auto r = PlanFile::readFromFile(loaded, p);
    if (!r.ok()) {
        setStatus(QStringLiteral("Load failed: %1").arg(r.message));
        emit errorMessage(r.message);
        return false;
    }
    if (missionPreviewActive()) {
        m_previewMockVehicle->stopMissionPreview(QStringLiteral("Mission loaded"));
    }
    m_plan = loaded;
    m_plan.renumberSequencesInPlace();
    m_dirty = false;
    m_currentFilePath = p;
    m_validationRun = false;
    m_validationErrors.clear();
    m_validationWarnings.clear();
    setSelectedIndex(m_plan.items.isEmpty() ? -1 : 0);
    emit itemsChanged();
    emit missionPreviewChanged();
    emit dirtyChanged();
    emit currentFilePathChanged();
    emit validationChanged();
    setStatus(QStringLiteral("Loaded %1 (%2 items)").arg(p).arg(m_plan.items.size()));
    emit infoMessage(QStringLiteral("Loaded %1").arg(p));
    return true;
}

bool MissionViewModel::uploadToVehicle()
{
    if (!m_activeManager || !m_activeVehicle) {
        setStatus(QStringLiteral("Upload blocked: no mission link is wired for the active vehicle."));
        emit errorMessage(QStringLiteral("No active mission link is available for upload."));
        return false;
    }
    if (m_plan.items.isEmpty()) {
        setStatus(QStringLiteral("Upload blocked: add at least one waypoint first."));
        emit errorMessage(QStringLiteral("Mission is empty; add waypoints before upload."));
        return false;
    }
    // SAFETY (Phase 8): SafetyGate is the single source of truth for whether
    // a mission upload may proceed. The Mock vehicle and SITL transports are
    // allowed; hardware (LinkKind::Serial) is blocked here even though the
    // QML button is also hidden — defence in depth.
    gcs::safety::SafetyGate gate;
    const auto d = gate.canUploadMission(m_activeVehicle->stateStore()->state(),
                                         m_plan);
    if (!d.allowed) {
        setStatus(QStringLiteral("Upload blocked: %1").arg(d.reason));
        emit errorMessage(d.reason);
        return false;
    }
    return m_activeManager->startUpload(m_plan);
}

bool MissionViewModel::downloadFromVehicle()
{
    if (!m_activeManager || !m_activeVehicle) {
        setStatus(QStringLiteral("Download blocked: no mission link is wired for the active vehicle."));
        emit errorMessage(QStringLiteral("No active mission link is available for download."));
        return false;
    }
    // Download symmetry: real-hardware download is blocked for the same
    // reason as upload — we have not validated the round-trip integrity on
    // a real autopilot in this phase, and the lab posture is read-only.
    if (m_activeVehicle->stateStore()->state().linkKind == gcs::comms::LinkKind::Serial) {
        const QString msg = QStringLiteral(
            "Mission download from real-hardware serial link is disabled; hardware mode is read-only.");
        setStatus(msg);
        emit errorMessage(msg);
        return false;
    }
    if (!linkFreshForTransfer(m_activeVehicle->stateStore()->state())) {
        const QString msg = QStringLiteral(
            "Mission download blocked: link is disconnected or heartbeat is stale.");
        setStatus(msg);
        emit errorMessage(msg);
        return false;
    }
    return m_activeManager->startDownload();
}

void MissionViewModel::cancelTransfer()
{
    if (m_activeManager) m_activeManager->cancel(QStringLiteral("Cancelled by user"));
}

bool MissionViewModel::simulateMission()
{
    if (missionPreviewActive()) {
        setStatus(QStringLiteral("Mission preview is already running."));
        return true;
    }

    if (!missionPreviewAllowed()) {
        const QString reason = missionPreviewBlockedReason();
        setStatus(QStringLiteral("Mission preview blocked: %1").arg(reason));
        emit errorMessage(reason);
        return false;
    }

    MissionValidator v;
    auto* fw = activeFirmware();
    const ValidationResult r = v.validate(m_plan, fw);
    m_validationErrors   = r.errorMessages();
    m_validationWarnings = r.warningMessages();
    m_validationRun      = true;
    emit validationChanged();
    if (!r.isValid()) {
        const QString msg = QStringLiteral(
            "Mission preview blocked: validation failed (%1 error(s)).")
            .arg(m_validationErrors.size());
        setStatus(msg);
        emit errorMessage(msg);
        return false;
    }

    const bool ok = m_previewMockVehicle->startMissionPreview(
        m_plan, m_plan.cruiseSpeedMps);
    if (!ok) {
        const QString msg = QStringLiteral("Mission preview failed to start.");
        setStatus(msg);
        emit errorMessage(msg);
        return false;
    }

    m_missionPreviewStatus = QStringLiteral("Running");
    m_missionPreviewProgress = 0.0;
    setStatus(QStringLiteral("Mission preview started (%1 items)")
              .arg(m_plan.items.size()));
    emit infoMessage(QStringLiteral("Mission preview started (%1 items)")
                     .arg(m_plan.items.size()));
    emit missionPreviewStarted();
    emit missionPreviewChanged();
    return true;
}

void MissionViewModel::stopMissionSimulation()
{
    if (!missionPreviewActive()) {
        return;
    }
    m_previewMockVehicle->stopMissionPreview(QStringLiteral("Stopped by operator"));
    emit infoMessage(QStringLiteral("Mission preview stopped by operator"));
}

QString MissionViewModel::vehicleLabel() const
{
    if (!m_activeVehicle) return QStringLiteral("(no vehicle)");
    const auto& s = m_activeVehicle->stateStore()->state();
    return QStringLiteral("%1 #%2").arg(s.vehicleType).arg(s.systemId);
}

bool MissionViewModel::vehicleSimulated() const
{
    return m_activeVehicle && m_activeVehicle->stateStore()->state().simulated;
}

bool MissionViewModel::hasMissionManager() const
{
    return m_activeManager != nullptr;
}

QString MissionViewModel::transferTarget() const
{
    if (!m_activeVehicle) return QStringLiteral("(no vehicle)");
    const auto& s = m_activeVehicle->stateStore()->state();
    if (s.simulated) return QStringLiteral("MockVehicle");
    switch (s.linkKind) {
        case gcs::comms::LinkKind::Mock:
            return QStringLiteral("MockVehicle");
        case gcs::comms::LinkKind::Udp:
        case gcs::comms::LinkKind::Tcp: {
            const QString transport =
                (s.linkKind == gcs::comms::LinkKind::Udp) ? "UDP" : "TCP";
            if (s.autopilotType.compare("PX4",       Qt::CaseInsensitive) == 0)
                return QStringLiteral("PX4 SITL (%1)").arg(transport);
            if (s.autopilotType.compare("ArduPilot", Qt::CaseInsensitive) == 0)
                return QStringLiteral("ArduPilot SITL (%1)").arg(transport);
            return QStringLiteral("Unknown SITL (%1)").arg(transport);
        }
        case gcs::comms::LinkKind::Serial:
            return QStringLiteral("Hardware Read-Only (serial)");
        case gcs::comms::LinkKind::Replay:
            return QStringLiteral("Replay");
        case gcs::comms::LinkKind::Unknown:
            return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

bool MissionViewModel::transferAllowed() const
{
    if (!m_activeManager || !m_activeVehicle) return false;
    const auto& s = m_activeVehicle->stateStore()->state();
    if (!linkFreshForTransfer(s)) return false;
    // Mirror SafetyGate::canUploadMission, but without requiring a populated
    // plan — this is for enabling/disabling the QML buttons.
    if (s.simulated) return true;
    switch (s.linkKind) {
        case gcs::comms::LinkKind::Mock:
            return true;
        case gcs::comms::LinkKind::Udp:
        case gcs::comms::LinkKind::Tcp:
            return s.autopilotType.compare("PX4",       Qt::CaseInsensitive) == 0
                || s.autopilotType.compare("ArduPilot", Qt::CaseInsensitive) == 0;
        case gcs::comms::LinkKind::Serial:
        case gcs::comms::LinkKind::Replay:
        case gcs::comms::LinkKind::Unknown:
            return false;
    }
    return false;
}

QString MissionViewModel::transferBlockedReason() const
{
    if (!m_activeVehicle)  return QStringLiteral("No active vehicle.");
    if (!m_activeManager)  return QStringLiteral("No mission link is wired for this vehicle.");
    const auto& s = m_activeVehicle->stateStore()->state();
    if (s.linkKind == gcs::comms::LinkKind::Serial) {
        return QStringLiteral(
            "Mission transfer to real hardware is disabled; hardware mode is read-only.");
    }
    if (!linkFreshForTransfer(s)) {
        return QStringLiteral(
            "Mission transfer requires a connected link with a fresh heartbeat.");
    }
    if (transferAllowed()) return QString();
    return QStringLiteral(
        "Mission transfer requires a Mock vehicle or PX4/ArduPilot SITL link.");
}

bool MissionViewModel::missionPreviewActive() const
{
    return m_previewMockVehicle && m_previewMockVehicle->missionPreviewActive();
}

bool MissionViewModel::missionPreviewAllowed() const
{
    if (!m_previewMockVehicle || !m_activeVehicle) return false;
    if (m_activeVehicle != m_previewMockVehicle->vehicle()) return false;
    if (m_plan.items.isEmpty()) return false;
    const auto& s = m_activeVehicle->stateStore()->state();
    return s.simulated && linkFreshForTransfer(s);
}

QString MissionViewModel::missionPreviewBlockedReason() const
{
    if (!m_previewMockVehicle) return QStringLiteral("Mock mission preview is not wired.");
    if (!m_activeVehicle) return QStringLiteral("No active vehicle.");
    if (m_activeVehicle != m_previewMockVehicle->vehicle()) {
        return QStringLiteral("Mission preview is mock-only; switch to Mock vehicle first.");
    }
    if (m_plan.items.isEmpty()) return QStringLiteral("Add waypoints before preview.");
    const auto& s = m_activeVehicle->stateStore()->state();
    if (!s.simulated) return QStringLiteral("Mission preview only runs on MockVehicle.");
    if (!linkFreshForTransfer(s)) {
        return QStringLiteral("MockVehicle heartbeat is not fresh.");
    }
    return QString();
}

void MissionViewModel::onActiveVehicleChanged(gcs::vehicle::Vehicle* v)
{
    if (m_activeManager) {
        disconnect(m_activeManager, nullptr, this, nullptr);
    }
    if (m_previewMockVehicle
        && v != m_previewMockVehicle->vehicle()
        && m_previewMockVehicle->missionPreviewActive()) {
        m_previewMockVehicle->stopMissionPreview(
            QStringLiteral("Active vehicle changed"));
    }
    m_activeVehicle = v;
    m_activeManager = v ? v->missionManager() : nullptr;
    rebindMissionManager();
    emit vehicleChanged();
    emit missionPreviewChanged();
}

void MissionViewModel::rebindMissionManager()
{
    if (!m_activeManager) return;
    connect(m_activeManager, &gcs::mission::MissionManager::uploadStarted,
            this, &MissionViewModel::onUploadStarted);
    connect(m_activeManager, &gcs::mission::MissionManager::uploadProgress,
            this, &MissionViewModel::onUploadProgress);
    connect(m_activeManager, &gcs::mission::MissionManager::uploadCompleted,
            this, &MissionViewModel::onUploadCompleted);
    connect(m_activeManager, &gcs::mission::MissionManager::downloadStarted,
            this, &MissionViewModel::onDownloadStarted);
    connect(m_activeManager, &gcs::mission::MissionManager::downloadProgress,
            this, &MissionViewModel::onDownloadProgress);
    connect(m_activeManager, &gcs::mission::MissionManager::downloadCompleted,
            this, &MissionViewModel::onDownloadCompleted);
    connect(m_activeManager, &gcs::mission::MissionManager::rejected,
            this, &MissionViewModel::onTransferRejected);
}

void MissionViewModel::onUploadStarted(int total)
{
    m_transferBusy    = true;
    m_transferLabel   = QStringLiteral("Uploading");
    m_transferCurrent = 0;
    m_transferTotal   = total;
    emit transferChanged();
    setStatus(QStringLiteral("Upload started (%1 items)").arg(total));
}

void MissionViewModel::onUploadProgress(int sent, int total)
{
    m_transferCurrent = sent;
    m_transferTotal   = total;
    emit transferChanged();
}

void MissionViewModel::onUploadCompleted(bool success, const QString& msg)
{
    resetTransferState();
    setStatus(QStringLiteral("Upload %1: %2")
              .arg(success ? "OK" : "FAILED", msg));
    if (success) emit infoMessage(msg);
    else         emit errorMessage(msg);
}

void MissionViewModel::onDownloadStarted()
{
    m_transferBusy    = true;
    m_transferLabel   = QStringLiteral("Downloading");
    m_transferCurrent = 0;
    m_transferTotal   = 0;
    emit transferChanged();
    setStatus(QStringLiteral("Download started"));
}

void MissionViewModel::onDownloadProgress(int got, int total)
{
    m_transferCurrent = got;
    m_transferTotal   = total;
    emit transferChanged();
}

void MissionViewModel::onDownloadCompleted(bool success, const QString& msg,
                                           const MissionPlan& plan)
{
    resetTransferState();
    if (success) {
        if (missionPreviewActive()) {
            m_previewMockVehicle->stopMissionPreview(QStringLiteral("Mission downloaded"));
        }
        m_plan = plan;
        m_plan.renumberSequencesInPlace();
        m_dirty = false;
        m_validationRun = false;
        m_validationErrors.clear();
        m_validationWarnings.clear();
        setSelectedIndex(m_plan.items.isEmpty() ? -1 : 0);
        emit itemsChanged();
        emit missionPreviewChanged();
        emit dirtyChanged();
        emit validationChanged();
        emit infoMessage(msg);
    } else {
        emit errorMessage(msg);
    }
    setStatus(QStringLiteral("Download %1: %2")
              .arg(success ? "OK" : "FAILED", msg));
}

void MissionViewModel::onTransferRejected(const QString& reason)
{
    setStatus(QStringLiteral("Transfer rejected: %1").arg(reason));
    emit errorMessage(reason);
}

void MissionViewModel::onMissionPreviewProgress(int currentIndex,
                                                int totalItems,
                                                double progress)
{
    Q_UNUSED(currentIndex)
    Q_UNUSED(totalItems)
    m_missionPreviewStatus = missionPreviewActive()
        ? QStringLiteral("Running")
        : m_missionPreviewStatus;
    m_missionPreviewProgress = progress;
    emit missionPreviewChanged();
}

void MissionViewModel::onMissionPreviewCompleted(int totalItems)
{
    m_missionPreviewStatus = QStringLiteral("Completed");
    m_missionPreviewProgress = 1.0;
    setStatus(QStringLiteral("Mission preview completed (%1 items)")
              .arg(totalItems));
    emit missionPreviewChanged();
}

void MissionViewModel::onMissionPreviewStopped(const QString& reason)
{
    m_missionPreviewStatus = QStringLiteral("Stopped");
    setStatus(QStringLiteral("Mission preview stopped: %1").arg(reason));
    emit missionPreviewChanged();
}

void MissionViewModel::markDirty()
{
    if (missionPreviewActive()) {
        m_previewMockVehicle->stopMissionPreview(QStringLiteral("Mission edited"));
    }
    if (m_dirty) return;
    m_dirty = true;
    emit dirtyChanged();
}

void MissionViewModel::setStatus(const QString& text)
{
    if (m_statusText == text) return;
    m_statusText = text;
    emit statusChanged();
}

void MissionViewModel::resetTransferState()
{
    m_transferBusy    = false;
    m_transferLabel.clear();
    m_transferCurrent = 0;
    m_transferTotal   = 0;
    emit transferChanged();
}

gcs::firmware::FirmwarePlugin* MissionViewModel::activeFirmware() const
{
    return m_activeVehicle ? m_activeVehicle->firmwarePlugin() : nullptr;
}

} // namespace gcs::viewmodels
