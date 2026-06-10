import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Mission Actions")

    readonly property int compactButtonHeight: 34

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
        spacing: Theme.gapSm

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Theme.gapSm
            rowSpacing: Theme.gapSm

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                radius: Theme.radiusSm
                color: missionVm.transferAllowed ? Theme.successSurface : Theme.dangerSurface
                border.color: missionVm.transferAllowed ? Theme.successBorder : Theme.dangerBorder

                Label {
                    anchors.fill: parent
                    anchors.margins: 8
                    text: missionVm.transferAllowed
                          ? qsTr("Target: ") + missionVm.transferTarget
                          : qsTr("Blocked: ") + missionVm.transferBlockedReason
                    color: missionVm.transferAllowed ? Theme.success : Theme.warning
                    font.bold: true
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                radius: Theme.radiusSm
                color: missionVm.missionPreviewActive
                       ? Theme.infoSurface
                       : (missionVm.missionPreviewAllowed ? Theme.successSurface : Theme.dangerSurface)
                border.color: missionVm.missionPreviewActive
                              ? Theme.accent
                              : (missionVm.missionPreviewAllowed ? Theme.successBorder : Theme.dangerBorder)

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Label {
                        Layout.fillWidth: true
                        text: missionVm.missionPreviewActive
                              ? qsTr("Preview: ") + Math.round(missionVm.missionPreviewProgress * 100) + "%"
                              : (missionVm.missionPreviewAllowed
                                 ? qsTr("Preview: MockVehicle")
                                 : qsTr("Preview blocked"))
                        color: missionVm.missionPreviewActive
                               ? Theme.blue
                               : (missionVm.missionPreviewAllowed ? Theme.success : Theme.warning)
                        font.bold: true
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Rectangle {
                        visible: missionVm.missionPreviewActive
                        Layout.fillWidth: true
                        height: 4
                        radius: 2
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

            Label {
                visible: !missionVm.missionPreviewActive && !missionVm.missionPreviewAllowed
                Layout.columnSpan: 2
                Layout.fillWidth: true
                text: qsTr("Preview blocked: ") + missionVm.missionPreviewBlockedReason
                color: Theme.warning
                wrapMode: Text.Wrap
                font.pixelSize: 11
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 6
            rowSpacing: 6

            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Validate")
                variant: "primary"
                onClicked: missionVm.validateMission()
            }
            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Simulate")
                variant: "primary"
                enabled: !missionVm.missionPreviewActive
                         && missionVm.missionPreviewAllowed
                         && missionVm.itemCount > 0
                onClicked: missionVm.simulateMission()
            }
            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Stop sim")
                variant: "danger"
                enabled: missionVm.missionPreviewActive
                onClicked: missionVm.stopMissionSimulation()
            }

            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Upload")
                variant: "primary"
                enabled: !missionVm.transferBusy
                         && missionVm.transferAllowed
                         && missionVm.itemCount > 0
                onClicked: missionVm.uploadToVehicle()
            }
            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Download")
                enabled: !missionVm.transferBusy
                         && missionVm.transferAllowed
                onClicked: missionVm.downloadFromVehicle()
            }
            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Cancel")
                variant: "danger"
                enabled: missionVm.transferBusy
                onClicked: missionVm.cancelTransfer()
            }

            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Save")
                enabled: missionVm.itemCount > 0
                onClicked: saveDialog.open()
            }
            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Load")
                onClicked: loadDialog.open()
            }
            StyledButton {
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactButtonHeight
                text: qsTr("Clear")
                variant: "danger"
                enabled: missionVm.itemCount > 0
                onClicked: missionVm.clearMission()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.border
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Theme.gapSm
            rowSpacing: 4

            Label {
                visible: missionVm.itemCount === 0
                Layout.columnSpan: 2
                Layout.fillWidth: true
                text: qsTr("Empty plan: add waypoints before validation or upload.")
                color: Theme.warning
                elide: Text.ElideRight
                font.pixelSize: 12
            }

            Label {
                visible: missionVm.transferAllowed && !missionVm.vehicleSimulated
                Layout.columnSpan: 2
                Layout.fillWidth: true
                text: qsTr("SITL transfer only. Upload/download does not arm, take off, start mission, land, or RTL.")
                color: Theme.textSecondary
                elide: Text.ElideRight
                font.pixelSize: 11
                font.italic: true
            }

            Label {
                text: qsTr("Vehicle")
                color: Theme.textMuted
                font.pixelSize: 11
            }
            Label {
                Layout.fillWidth: true
                text: missionVm.vehicleLabel
                      + (missionVm.vehicleSimulated ? qsTr(" (simulation)") : "")
                color: missionVm.vehicleSimulated ? Theme.warning : Theme.textPrimary
                elide: Text.ElideRight
                font.pixelSize: 12
            }

            Label {
                text: qsTr("File")
                color: Theme.textMuted
                font.pixelSize: 11
            }
            Label {
                Layout.fillWidth: true
                text: missionVm.currentFilePath !== ""
                      ? missionVm.currentFilePath + (missionVm.dirty ? qsTr(" (modified)") : "")
                      : qsTr("unsaved plan") + (missionVm.dirty ? qsTr(" (modified)") : "")
                color: Theme.textSecondary
                elide: Text.ElideMiddle
                font.pixelSize: 11
            }
        }
    }
}
