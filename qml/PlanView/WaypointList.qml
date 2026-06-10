import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Waypoints")

    signal editRequested(int index)

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
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 10
                    Label { text: qsTr("Seq"); color: Theme.textMuted; Layout.preferredWidth: 36; font.pixelSize: 11 }
                    Label { text: qsTr("Command"); color: Theme.textMuted; Layout.preferredWidth: 112; font.pixelSize: 11 }
                    Label { text: qsTr("Coordinates"); color: Theme.textMuted; Layout.fillWidth: true; font.pixelSize: 11 }
                    Label { text: qsTr("Alt"); color: Theme.textMuted; Layout.preferredWidth: 74; font.pixelSize: 11 }
                    Label { text: qsTr("Edit"); color: Theme.textMuted; Layout.preferredWidth: 58; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                }
            }

            delegate: Rectangle {
                width: list.width
                height: 42
                color: "transparent"

                MouseArea {
                    anchors.fill: parent
                    anchors.rightMargin: 70
                    onClicked: missionVm.selectedIndex = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 10
                    Label {
                        text: "#" + (modelData.seq + 1)
                        color: Theme.textSecondary
                        Layout.preferredWidth: 36
                    }
                    Label {
                        text: modelData.commandName
                        color: "white"
                        font.bold: true
                        Layout.preferredWidth: 112
                        elide: Text.ElideRight
                    }
                    Label {
                        text: Number(modelData.latitudeDeg).toFixed(5) + ", "
                              + Number(modelData.longitudeDeg).toFixed(5)
                        color: Theme.textPrimary
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Label {
                        text: Number(modelData.altitudeM).toFixed(1) + " m"
                        color: Theme.textSecondary
                        Layout.preferredWidth: 74
                        elide: Text.ElideRight
                    }
                    StyledButton {
                        text: qsTr("Edit")
                        Layout.preferredWidth: 58
                        Layout.preferredHeight: 30
                        horizontalPadding: 8
                        verticalPadding: 4
                        onClicked: {
                            missionVm.selectedIndex = index
                            root.editRequested(index)
                        }
                    }
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
