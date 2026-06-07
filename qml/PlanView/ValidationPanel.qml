import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Validation & Transfer Status")

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 8

        Rectangle {
            visible: missionVm.transferBusy
            Layout.fillWidth: true
            height: 46
            radius: 4
            color: Theme.infoSurface
            border.color: Theme.infoBorder
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4
                Label {
                    text: missionVm.transferLabel + " "
                          + missionVm.transferCurrent + " / "
                          + Math.max(1, missionVm.transferTotal)
                    color: "white"
                    font.bold: true
                    Layout.fillWidth: true
                }
                StyledProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: Math.max(1, missionVm.transferTotal)
                    value: missionVm.transferCurrent
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 34
            radius: 4
            color: !missionVm.validationRun ? Theme.surfaceElevated
                  : missionVm.valid ? Theme.successSurface : Theme.dangerSurface
            border.color: !missionVm.validationRun ? Theme.border
                        : missionVm.valid ? Theme.successBorder : Theme.dangerBorder
            Label {
                anchors.centerIn: parent
                text: !missionVm.validationRun
                      ? qsTr("Validation not run")
                      : missionVm.valid
                        ? qsTr("Mission valid")
                          + (missionVm.validationWarnings.length > 0
                             ? " - " + missionVm.validationWarnings.length + qsTr(" warning(s)")
                             : "")
                        : qsTr("Mission invalid - ")
                          + missionVm.validationErrors.length
                          + qsTr(" error(s)")
                color: !missionVm.validationRun ? Theme.textPrimary
                      : missionVm.valid ? Theme.success : Theme.danger
                font.bold: true
            }
        }

        ListView {
            id: validationList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: missionVm.validationErrors.concat(missionVm.validationWarnings)
            delegate: Rectangle {
                width: validationList.width
                implicitHeight: msg.implicitHeight + 8
                color: index % 2 === 0 ? Theme.listRow : Theme.listRowAlt
                Label {
                    id: msg
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    text: (index < missionVm.validationErrors.length
                           ? "ERROR: " : "WARN: ") + modelData
                    color: index < missionVm.validationErrors.length
                           ? Theme.danger : Theme.warning
                    wrapMode: Text.WrapAnywhere
                    font.pixelSize: 12
                }
            }

            Label {
                anchors.centerIn: parent
                visible: validationList.count === 0
                         && !missionVm.transferBusy
                text: missionVm.validationRun
                      ? qsTr("No validation messages.")
                      : qsTr("Run validation to check mission altitude, frames, commands, and coordinates.")
                color: Theme.textMuted
                font.italic: true
                wrapMode: Text.Wrap
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        Label {
            text: qsTr("Last status: ") + missionVm.statusText
            color: Theme.textPrimary
            wrapMode: Text.WrapAnywhere
            Layout.fillWidth: true
        }
    }
}
