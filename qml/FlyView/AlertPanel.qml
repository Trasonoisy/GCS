import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Event Log")

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Theme.gapSm

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Runtime audit events")
                color: Theme.textPrimary
                font.bold: true
                Layout.fillWidth: true
            }
            Label {
                text: logVm.eventCount + qsTr(" entries")
                color: Theme.textSecondary
                font.pixelSize: 12
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AsNeeded }

            model: logVm.recentEvents
            delegate: Rectangle {
                width: list.width
                implicitHeight: line.implicitHeight + 8
                color: index % 2 === 0 ? Theme.listRow : Theme.listRowAlt
                Label {
                    id: line
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    text: modelData
                    color: modelData.indexOf("WARN/") >= 0 ? Theme.warning
                          : modelData.indexOf("ERROR/") >= 0 ? Theme.danger
                          : Theme.textPrimary
                    wrapMode: Text.WrapAnywhere
                    font.pixelSize: 12
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No audit events yet. Connect a link, validate a mission, or toggle manual control.")
                color: Theme.textMuted
                font.italic: true
                wrapMode: Text.Wrap
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
            }

            onCountChanged: positionViewAtEnd()
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: qsTr("JSONL: ") + (logVm.currentLogPath !== ""
                                      ? logVm.currentLogPath
                                      : qsTr("(file logging disabled)"))
                color: logVm.currentLogPath !== "" ? Theme.textSecondary : Theme.warning
                font.pixelSize: 11
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                StyledButton {
                    text: qsTr("Open folder")
                    Layout.fillWidth: true
                    enabled: logVm.currentLogPath !== ""
                    onClicked: logVm.openLogFolder()
                }
                StyledButton {
                    text: qsTr("Copy path")
                    Layout.fillWidth: true
                    enabled: logVm.currentLogPath !== ""
                    onClicked: logVm.copyLogPath()
                }
            }
        }
    }
}
