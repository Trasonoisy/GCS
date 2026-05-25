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
            color: "#1F3A5F"
            border.color: "#264F78"
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
                ProgressBar {
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
            color: !missionVm.validationRun ? "#252525"
                  : missionVm.valid ? "#19351F" : "#3A2424"
            border.color: !missionVm.validationRun ? "#3C3C3C"
                        : missionVm.valid ? "#2C703A" : "#7A3A3A"
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
                color: !missionVm.validationRun ? "#CCCCCC"
                      : missionVm.valid ? "#A0E060" : "#FF8080"
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
                color: index % 2 === 0 ? "#181818" : "#202020"
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
                           ? "#FF8080" : "#FFCC66"
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
                color: "#777777"
                font.italic: true
                wrapMode: Text.Wrap
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#3C3C3C" }

        Label {
            text: qsTr("Last status: ") + missionVm.statusText
            color: "#CCCCCC"
            wrapMode: Text.WrapAnywhere
            Layout.fillWidth: true
        }
    }
}
