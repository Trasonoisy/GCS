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
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Runtime audit events")
                color: "#CCCCCC"
                font.bold: true
                Layout.fillWidth: true
            }
            Label {
                text: logVm.eventCount + qsTr(" entries")
                color: "#9A9A9A"
                font.pixelSize: 12
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: logVm.recentEvents
            delegate: Rectangle {
                width: list.width
                implicitHeight: line.implicitHeight + 8
                color: index % 2 === 0 ? "#181818" : "#202020"
                Label {
                    id: line
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    text: modelData
                    color: modelData.indexOf("WARN/") >= 0 ? "#FFCC66"
                          : modelData.indexOf("ERROR/") >= 0 ? "#FF8080"
                          : "#DDDDDD"
                    wrapMode: Text.WrapAnywhere
                    font.pixelSize: 12
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No audit events yet. Connect a link, validate a mission, or toggle manual control.")
                color: "#777777"
                font.italic: true
                wrapMode: Text.Wrap
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
            }

            onCountChanged: positionViewAtEnd()
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#3C3C3C" }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: qsTr("JSONL: ") + (logVm.currentLogPath !== ""
                                      ? logVm.currentLogPath
                                      : qsTr("(file logging disabled)"))
                color: logVm.currentLogPath !== "" ? "#9A9A9A" : "#FFAA33"
                font.pixelSize: 11
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
            }
            Button {
                text: qsTr("Open folder")
                enabled: logVm.currentLogPath !== ""
                onClicked: logVm.openLogFolder()
            }
            Button {
                text: qsTr("Copy path")
                enabled: logVm.currentLogPath !== ""
                onClicked: logVm.copyLogPath()
            }
        }
    }
}
