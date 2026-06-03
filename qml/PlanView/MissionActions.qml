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
            color: missionVm.transferAllowed ? "#19351F" : "#3A2424"
            border.color: missionVm.transferAllowed ? "#2C703A" : "#7A3A3A"
            Label {
                anchors.centerIn: parent
                width: parent.width - 16
                text: missionVm.transferAllowed
                      ? qsTr("Transfer target: ") + missionVm.transferTarget
                      : qsTr("Transfer blocked: ") + missionVm.transferBlockedReason
                color: missionVm.transferAllowed ? "#A0E060" : "#FFAA33"
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
                   ? "#12283D"
                   : (missionVm.missionPreviewAllowed ? "#202A24" : "#2A2424")
            border.color: missionVm.missionPreviewActive
                          ? "#1F6FEB"
                          : (missionVm.missionPreviewAllowed ? "#3B6A4A" : "#5A4545")

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
                           ? "#7DB7FF"
                           : (missionVm.missionPreviewAllowed ? "#A0E060" : "#FFAA33")
                    font.bold: true
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Rectangle {
                    visible: missionVm.missionPreviewActive
                    Layout.fillWidth: true
                    height: 6
                    radius: 3
                    color: "#1F1F1F"
                    Rectangle {
                        width: parent.width * missionVm.missionPreviewProgress
                        height: parent.height
                        radius: parent.radius
                        color: "#1F6FEB"
                    }
                }
            }
        }

        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 8
            Layout.fillWidth: true

            Button {
                Layout.fillWidth: true
                text: qsTr("Validate mission")
                onClicked: missionVm.validateMission()
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Save plan")
                enabled: missionVm.itemCount > 0
                onClicked: saveDialog.open()
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Load plan")
                onClicked: loadDialog.open()
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Clear plan")
                enabled: missionVm.itemCount > 0
                onClicked: missionVm.clearMission()
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Simulate mission")
                enabled: !missionVm.missionPreviewActive
                         && missionVm.missionPreviewAllowed
                         && missionVm.itemCount > 0
                onClicked: missionVm.simulateMission()
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Stop simulation")
                enabled: missionVm.missionPreviewActive
                onClicked: missionVm.stopMissionSimulation()
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("Upload to ") + missionVm.transferTarget
                enabled: !missionVm.transferBusy
                         && missionVm.transferAllowed
                         && missionVm.itemCount > 0
                onClicked: missionVm.uploadToVehicle()
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Download from ") + missionVm.transferTarget
                enabled: !missionVm.transferBusy
                         && missionVm.transferAllowed
                onClicked: missionVm.downloadFromVehicle()
            }

            Button {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Cancel transfer")
                enabled: missionVm.transferBusy
                onClicked: missionVm.cancelTransfer()
            }
        }

        Label {
            visible: missionVm.itemCount === 0
            text: qsTr("Empty plan: add waypoints before validation or upload.")
            color: "#FFAA33"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 12
        }

        Label {
            visible: missionVm.transferAllowed && !missionVm.vehicleSimulated
            text: qsTr("SITL mission transfer only. Upload/download does not arm, take off, change mode, start mission, land, or RTL.")
            color: "#9A9A9A"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 11
            font.italic: true
        }

        Label {
            text: qsTr("Vehicle: ") + missionVm.vehicleLabel
                  + (missionVm.vehicleSimulated ? qsTr(" (simulation)") : "")
            color: missionVm.vehicleSimulated ? "#FFAA33" : "#CCCCCC"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Label {
            text: missionVm.currentFilePath !== ""
                  ? qsTr("File: ") + missionVm.currentFilePath
                                   + (missionVm.dirty ? qsTr(" (modified)") : "")
                  : qsTr("File: unsaved plan") + (missionVm.dirty ? qsTr(" (modified)") : "")
            color: "#9A9A9A"
            wrapMode: Text.WrapAnywhere
            Layout.fillWidth: true
            font.pixelSize: 12
        }
    }
}
