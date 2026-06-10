#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "Mission/MissionPlan.h"

namespace gcs::firmware { class FirmwarePlugin; }
namespace gcs::mission  { class MissionManager; }
namespace gcs::simulation { class MockVehicle; }
namespace gcs::vehicle  { class MultiVehicleManager; class Vehicle; }

namespace gcs::viewmodels {

// QML-facing adapter for the mission editor. Owns the working-copy
// MissionPlan (m_plan). Talks to the active Vehicle's MissionManager for
// upload/download. Surfaces validation results, transfer progress, and
// transfer state for the PlanView UI to bind against.
//
// SAFETY: This is the *only* QML-reachable entry point for mission
// transfer. Upload/download routes through MissionManager, which routes
// through IMissionLink — in Phase 2 always a MockMissionLink.
class MissionViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int          itemCount         READ itemCount         NOTIFY itemsChanged)
    Q_PROPERTY(QVariantList items             READ items             NOTIFY itemsChanged)
    Q_PROPERTY(int          selectedIndex     READ selectedIndex     WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QVariantMap  selectedItem      READ selectedItem      NOTIFY selectedItemChanged)
    Q_PROPERTY(bool         dirty             READ dirty             NOTIFY dirtyChanged)
    Q_PROPERTY(QString      currentFilePath   READ currentFilePath   NOTIFY currentFilePathChanged)

    Q_PROPERTY(QStringList  validationErrors   READ validationErrors   NOTIFY validationChanged)
    Q_PROPERTY(QStringList  validationWarnings READ validationWarnings NOTIFY validationChanged)
    Q_PROPERTY(bool         validationRun      READ validationRun      NOTIFY validationChanged)
    Q_PROPERTY(bool         valid              READ valid              NOTIFY validationChanged)

    Q_PROPERTY(bool         transferBusy      READ transferBusy      NOTIFY transferChanged)
    Q_PROPERTY(QString      transferLabel     READ transferLabel     NOTIFY transferChanged)
    Q_PROPERTY(int          transferCurrent   READ transferCurrent   NOTIFY transferChanged)
    Q_PROPERTY(int          transferTotal     READ transferTotal     NOTIFY transferChanged)
    Q_PROPERTY(QString      statusText        READ statusText        NOTIFY statusChanged)
    Q_PROPERTY(QString      vehicleLabel      READ vehicleLabel      NOTIFY vehicleChanged)
    Q_PROPERTY(bool         vehicleSimulated  READ vehicleSimulated  NOTIFY vehicleChanged)
    Q_PROPERTY(bool         hasMissionManager READ hasMissionManager NOTIFY vehicleChanged)
    // Phase 8: surface the kind of vehicle the editor is bound to so QML can
    // label the upload/download buttons accurately and disable them when the
    // active vehicle is real hardware.
    Q_PROPERTY(QString      transferTarget       READ transferTarget       NOTIFY vehicleChanged)
    Q_PROPERTY(bool         transferAllowed      READ transferAllowed      NOTIFY vehicleChanged)
    Q_PROPERTY(QString      transferBlockedReason READ transferBlockedReason NOTIFY vehicleChanged)
    Q_PROPERTY(bool         missionPreviewActive READ missionPreviewActive NOTIFY missionPreviewChanged)
    Q_PROPERTY(bool         missionPreviewAllowed READ missionPreviewAllowed NOTIFY missionPreviewChanged)
    Q_PROPERTY(QString      missionPreviewBlockedReason READ missionPreviewBlockedReason NOTIFY missionPreviewChanged)
    Q_PROPERTY(QString      missionPreviewStatus READ missionPreviewStatus NOTIFY missionPreviewChanged)
    Q_PROPERTY(double       missionPreviewProgress READ missionPreviewProgress NOTIFY missionPreviewChanged)
    Q_PROPERTY(int          missionPreviewCurrentIndex READ missionPreviewCurrentIndex NOTIFY missionPreviewChanged)

public:
    explicit MissionViewModel(gcs::vehicle::MultiVehicleManager* manager,
                              QObject* parent = nullptr);

    // Properties
    int          itemCount()      const { return m_plan.items.size(); }
    QVariantList items()          const;
    int          selectedIndex()  const { return m_selectedIndex; }
    void         setSelectedIndex(int i);
    QVariantMap  selectedItem()   const;
    bool         dirty()          const { return m_dirty; }
    QString      currentFilePath() const { return m_currentFilePath; }

    QStringList  validationErrors()   const { return m_validationErrors; }
    QStringList  validationWarnings() const { return m_validationWarnings; }
    bool         validationRun()      const { return m_validationRun; }
    bool         valid()              const { return m_validationRun && m_validationErrors.isEmpty(); }

    bool    transferBusy()    const { return m_transferBusy; }
    QString transferLabel()   const { return m_transferLabel; }
    int     transferCurrent() const { return m_transferCurrent; }
    int     transferTotal()   const { return m_transferTotal; }
    QString statusText()      const { return m_statusText; }
    QString vehicleLabel()    const;
    bool    vehicleSimulated() const;
    bool    hasMissionManager() const;
    QString transferTarget()       const;
    bool    transferAllowed()      const;
    QString transferBlockedReason() const;
    bool    missionPreviewActive() const;
    bool    missionPreviewAllowed() const;
    QString missionPreviewBlockedReason() const;
    QString missionPreviewStatus() const { return m_missionPreviewStatus; }
    double  missionPreviewProgress() const { return m_missionPreviewProgress; }
    int     missionPreviewCurrentIndex() const { return m_missionPreviewCurrentIndex; }

    void setMissionPreviewMockVehicle(gcs::simulation::MockVehicle* mockVehicle);

    // Methods callable from QML
    Q_INVOKABLE void addWaypoint();
    Q_INVOKABLE void deleteWaypoint(int index);
    Q_INVOKABLE void moveWaypoint(int fromIndex, int toIndex);
    Q_INVOKABLE void updateWaypointField(int index, const QString& field, const QVariant& value);
    Q_INVOKABLE void clearMission();
    Q_INVOKABLE void validateMission();

    // Path may be a local path or a "file:///..." URL — both are accepted.
    Q_INVOKABLE bool saveToFile(const QString& path);
    Q_INVOKABLE bool loadFromFile(const QString& path);

    Q_INVOKABLE bool uploadToVehicle();
    Q_INVOKABLE bool downloadFromVehicle();
    Q_INVOKABLE void cancelTransfer();
    Q_INVOKABLE bool simulateMission();
    Q_INVOKABLE void stopMissionSimulation();

signals:
    void itemsChanged();
    void selectedIndexChanged();
    void selectedItemChanged();
    void dirtyChanged();
    void currentFilePathChanged();
    void validationChanged();
    void transferChanged();
    void statusChanged();
    void vehicleChanged();
    void missionPreviewChanged();
    void missionPreviewStarted();
    void infoMessage(const QString& message);
    void errorMessage(const QString& message);

private slots:
    void onActiveVehicleChanged(gcs::vehicle::Vehicle* v);
    void onUploadStarted(int total);
    void onUploadProgress(int sent, int total);
    void onUploadCompleted(bool success, const QString& msg);
    void onDownloadStarted();
    void onDownloadProgress(int got, int total);
    void onDownloadCompleted(bool success, const QString& msg,
                             const gcs::mission::MissionPlan& plan);
    void onTransferRejected(const QString& reason);
    void onMissionPreviewProgress(int currentIndex, int totalItems, double progress);
    void onMissionPreviewCompleted(int totalItems);
    void onMissionPreviewStopped(const QString& reason);

private:
    QVariantMap itemToMap(const gcs::mission::MissionItem& it, int index) const;
    void rebindMissionManager();
    void markDirty();
    void setStatus(const QString& text);
    void resetTransferState();
    gcs::firmware::FirmwarePlugin* activeFirmware() const;

    gcs::vehicle::MultiVehicleManager* m_manager;
    gcs::vehicle::Vehicle*             m_activeVehicle = nullptr;
    gcs::mission::MissionManager*      m_activeManager = nullptr;
    gcs::simulation::MockVehicle*      m_previewMockVehicle = nullptr;

    gcs::mission::MissionPlan m_plan;
    int     m_selectedIndex = -1;
    bool    m_dirty         = false;
    QString m_currentFilePath;

    QStringList m_validationErrors;
    QStringList m_validationWarnings;
    bool        m_validationRun = false;

    bool    m_transferBusy    = false;
    QString m_transferLabel;
    int     m_transferCurrent = 0;
    int     m_transferTotal   = 0;
    QString m_statusText      = QStringLiteral("Ready");

    QString m_missionPreviewStatus = QStringLiteral("Idle");
    double  m_missionPreviewProgress = 0.0;
    int     m_missionPreviewCurrentIndex = -1;
};

} // namespace gcs::viewmodels
