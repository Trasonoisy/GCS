import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Mission Actions")

    FileDialog {
        id: saveDialog
        title: qsTr("Save mission")
        nameFilters: ["Plan files (*.plan)", "All files (*)"]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "plan"
        onAccepted: missionVm.saveToFile(saveDialog.selectedFile)
    }

    FileDialog {
        id: loadDialog
        title: qsTr("Load mission")
        nameFilters: ["Plan files (*.plan)", "All files (*)"]
        fileMode: FileDialog.OpenFile
        onAccepted: missionVm.loadFromFile(loadDialog.selectedFile)
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            height: 42
            radius: 4
            color: missionVm.transferAllowed ? Theme.successSurface : Theme.dangerSurface
            border.color: missionVm.transferAllowed ? Theme.successBorder : Theme.dangerBorder
            Label {
                anchors.centerIn: parent
                width: parent.width - 16
                text: missionVm.transferAllowed
                      ? qsTr("Transfer target: ") + missionVm.transferTarget
                      : qsTr("Transfer blocked: ") + missionVm.transferBlockedReason
                color: missionVm.transferAllowed ? Theme.success : Theme.warning
                font.bold: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: missionVm.missionPreviewActive ? 56 : 42
            radius: 4
            color: missionVm.missionPreviewActive
                   ? Theme.infoSurface
                   : (missionVm.missionPreviewAllowed ? Theme.successSurface : Theme.dangerSurface)
            border.color: missionVm.missionPreviewActive
                          ? Theme.accent
                          : (missionVm.missionPreviewAllowed ? Theme.successBorder : Theme.dangerBorder)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: missionVm.missionPreviewActive
                          ? qsTr("Preview running: ") + Math.round(missionVm.missionPreviewProgress * 100) + "%"
                          : (missionVm.missionPreviewAllowed
                             ? qsTr("Preview target: MockVehicle")
                             : qsTr("Preview blocked: ") + missionVm.missionPreviewBlockedReason)
                    color: missionVm.missionPreviewActive
                           ? Theme.blue
                           : (missionVm.missionPreviewAllowed ? Theme.success : Theme.warning)
                    font.bold: true
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Rectangle {
                    visible: missionVm.missionPreviewActive
                    Layout.fillWidth: true
                    height: 6
                    radius: 3
                    color: Theme.inputBackground
                    Rectangle {
                        width: parent.width * missionVm.missionPreviewProgress
                        height: parent.height
                        radius: parent.radius
                        color: Theme.accent
                    }
                }
            }
        }

        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 8
            Layout.fillWidth: true

            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Validate mission")
                variant: "primary"
                onClicked: missionVm.validateMission()
            }
            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Save plan")
                enabled: missionVm.itemCount > 0
                onClicked: saveDialog.open()
            }
            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Load plan")
                onClicked: loadDialog.open()
            }
            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Clear plan")
                enabled: missionVm.itemCount > 0
                onClicked: missionVm.clearMission()
            }

            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Simulate mission")
                variant: "primary"
                enabled: !missionVm.missionPreviewActive
                         && missionVm.missionPreviewAllowed
                         && missionVm.itemCount > 0
                onClicked: missionVm.simulateMission()
            }
            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Stop simulation")
                variant: "danger"
                enabled: missionVm.missionPreviewActive
                onClicked: missionVm.stopMissionSimulation()
            }

            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Upload to ") + missionVm.transferTarget
                variant: "primary"
                enabled: !missionVm.transferBusy
                         && missionVm.transferAllowed
                         && missionVm.itemCount > 0
                onClicked: missionVm.uploadToVehicle()
            }
            StyledButton {
                Layout.fillWidth: true
                text: qsTr("Download from ") + missionVm.transferTarget
                enabled: !missionVm.transferBusy
                         && missionVm.transferAllowed
                onClicked: missionVm.downloadFromVehicle()
            }

            StyledButton {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Cancel transfer")
                variant: "danger"
                enabled: missionVm.transferBusy
                onClicked: missionVm.cancelTransfer()
            }
        }

        Label {
            visible: missionVm.itemCount === 0
            text: qsTr("Empty plan: add waypoints before validation or upload.")
            color: Theme.warning
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 12
        }

        Label {
            visible: missionVm.transferAllowed && !missionVm.vehicleSimulated
            text: qsTr("SITL mission transfer only. Upload/download does not arm, take off, change mode, start mission, land, or RTL.")
            color: Theme.textSecondary
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 11
            font.italic: true
        }

        Label {
            text: qsTr("Vehicle: ") + missionVm.vehicleLabel
                  + (missionVm.vehicleSimulated ? qsTr(" (simulation)") : "")
            color: missionVm.vehicleSimulated ? Theme.warning : Theme.textPrimary
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Label {
            text: missionVm.currentFilePath !== ""
                  ? qsTr("File: ") + missionVm.currentFilePath
                                   + (missionVm.dirty ? qsTr(" (modified)") : "")
                  : qsTr("File: unsaved plan") + (missionVm.dirty ? qsTr(" (modified)") : "")
            color: Theme.textSecondary
            wrapMode: Text.WrapAnywhere
            Layout.fillWidth: true
            font.pixelSize: 12
        }
    }
}
