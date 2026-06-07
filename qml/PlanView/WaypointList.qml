import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Waypoints")

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 6

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: missionVm.items
            currentIndex: missionVm.selectedIndex
            highlightFollowsCurrentItem: true
            highlight: Rectangle { color: Theme.infoBorder; radius: 3 }

            header: Rectangle {
                width: list.width
                height: 24
                color: Theme.listRow
                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 12
                    Label { text: qsTr("Seq"); color: Theme.textMuted; width: 36; font.pixelSize: 11 }
                    Label { text: qsTr("Command"); color: Theme.textMuted; width: 110; font.pixelSize: 11 }
                    Label { text: qsTr("Coordinates"); color: Theme.textMuted; width: 190; font.pixelSize: 11 }
                    Label { text: qsTr("Alt"); color: Theme.textMuted; font.pixelSize: 11 }
                }
            }

            delegate: Rectangle {
                width: list.width
                height: 38
                color: "transparent"
                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 12
                    Label {
                        text: "#" + (modelData.seq + 1)
                        color: Theme.textSecondary
                        width: 36
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Label {
                        text: modelData.commandName
                        color: "white"
                        font.bold: true
                        width: 110
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                    }
                    Label {
                        text: Number(modelData.latitudeDeg).toFixed(5) + ", "
                              + Number(modelData.longitudeDeg).toFixed(5)
                        color: Theme.textPrimary
                        width: 190
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                    }
                    Label {
                        text: Number(modelData.altitudeM).toFixed(1) + " m"
                        color: Theme.textSecondary
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: missionVm.selectedIndex = index
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No waypoints yet. Click Add waypoint to create a safe plan.")
                color: Theme.textMuted
                font.italic: true
                wrapMode: Text.Wrap
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            StyledButton {
                text: qsTr("Add waypoint")
                variant: "primary"
                onClicked: missionVm.addWaypoint()
            }
            StyledButton {
                text: qsTr("Delete")
                variant: "danger"
                enabled: missionVm.selectedIndex >= 0
                onClicked: missionVm.deleteWaypoint(missionVm.selectedIndex)
            }
            StyledButton {
                text: qsTr("Move up")
                enabled: missionVm.selectedIndex > 0
                onClicked: missionVm.moveWaypoint(missionVm.selectedIndex,
                                                  missionVm.selectedIndex - 1)
            }
            StyledButton {
                text: qsTr("Move down")
                enabled: missionVm.selectedIndex >= 0
                         && missionVm.selectedIndex < missionVm.itemCount - 1
                onClicked: missionVm.moveWaypoint(missionVm.selectedIndex,
                                                  missionVm.selectedIndex + 1)
            }
        }
    }
}
