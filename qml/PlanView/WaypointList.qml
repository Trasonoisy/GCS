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
            highlight: Rectangle { color: "#264F78"; radius: 3 }

            header: Rectangle {
                width: list.width
                height: 24
                color: "#181818"
                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 12
                    Label { text: qsTr("Seq"); color: "#777777"; width: 36; font.pixelSize: 11 }
                    Label { text: qsTr("Command"); color: "#777777"; width: 110; font.pixelSize: 11 }
                    Label { text: qsTr("Coordinates"); color: "#777777"; width: 190; font.pixelSize: 11 }
                    Label { text: qsTr("Alt"); color: "#777777"; font.pixelSize: 11 }
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
                        color: "#9A9A9A"
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
                        color: "#CCCCCC"
                        width: 190
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                    }
                    Label {
                        text: Number(modelData.altitudeM).toFixed(1) + " m"
                        color: "#9A9A9A"
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
                color: "#777777"
                font.italic: true
                wrapMode: Text.Wrap
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Button {
                text: qsTr("Add waypoint")
                onClicked: missionVm.addWaypoint()
            }
            Button {
                text: qsTr("Delete")
                enabled: missionVm.selectedIndex >= 0
                onClicked: missionVm.deleteWaypoint(missionVm.selectedIndex)
            }
            Button {
                text: qsTr("Move up")
                enabled: missionVm.selectedIndex > 0
                onClicked: missionVm.moveWaypoint(missionVm.selectedIndex,
                                                  missionVm.selectedIndex - 1)
            }
            Button {
                text: qsTr("Move down")
                enabled: missionVm.selectedIndex >= 0
                         && missionVm.selectedIndex < missionVm.itemCount - 1
                onClicked: missionVm.moveWaypoint(missionVm.selectedIndex,
                                                  missionVm.selectedIndex + 1)
            }
        }
    }
}
