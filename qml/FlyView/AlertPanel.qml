import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Event Log")

    function eventColor(line) {
        return line.indexOf("WARN/") >= 0 ? Theme.warning
             : line.indexOf("ERROR/") >= 0 ? Theme.danger
             : Theme.textPrimary
    }

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
            Rectangle {
                id: expandButton
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                radius: 13
                color: expandMouse.containsMouse ? Theme.hoverSurface : Theme.surfaceElevated
                border.color: Theme.border
                border.width: 1

                Image {
                    anchors.centerIn: parent
                    width: 12
                    height: 12
                    source: "../assets/expand_content.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                }

                MouseArea {
                    id: expandMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: expandedLogDialog.open()
                    ToolTip.visible: containsMouse
                    ToolTip.text: qsTr("Expand event log")
                }
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
                    color: root.eventColor(modelData)
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

    Dialog {
        id: expandedLogDialog
        parent: Overlay.overlay
        modal: true
        focus: true
        dim: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        width: Math.max(720, Math.min(1080, (parent ? parent.width : root.width) - 80))
        height: Math.max(460, Math.min(760, (parent ? parent.height : root.height) - 80))
        x: parent ? Math.round((parent.width - width) / 2) : 0
        y: parent ? Math.round((parent.height - height) / 2) : 0

        background: Rectangle {
            radius: Theme.radiusLg
            color: Theme.surfaceRaised
            border.color: Theme.border
            border.width: 1
        }

        contentItem: ColumnLayout {
            spacing: Theme.gapMd

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.gapMd

                Label {
                    text: qsTr("Event Log")
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 20
                    Layout.fillWidth: true
                }

                Label {
                    text: logVm.eventCount + qsTr(" entries")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                }

                StyledButton {
                    text: qsTr("Close")
                    Layout.preferredWidth: 90
                    onClicked: expandedLogDialog.close()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Theme.border
            }

            ListView {
                id: expandedList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: logVm.recentEvents
                spacing: 2
                ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AsNeeded }

                delegate: Rectangle {
                    width: expandedList.width
                    implicitHeight: expandedLine.implicitHeight + 12
                    radius: Theme.radiusSm
                    color: index % 2 === 0 ? Theme.listRow : Theme.listRowAlt

                    Label {
                        id: expandedLine
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        text: modelData
                        color: root.eventColor(modelData)
                        wrapMode: Text.WrapAnywhere
                        font.family: Theme.monoFontFamily
                        font.pixelSize: 13
                        lineHeight: 1.15
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: expandedList.count === 0
                    text: qsTr("No audit events yet.")
                    color: Theme.textMuted
                    font.italic: true
                }

                onCountChanged: positionViewAtEnd()
            }
        }

        onOpened: expandedList.positionViewAtEnd()
    }
}
