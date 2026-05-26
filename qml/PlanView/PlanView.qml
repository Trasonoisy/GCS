import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: "#121212"

    ScrollView {
        id: planScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        contentHeight: planContent.y + planContent.implicitHeight + 12
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        RowLayout {
            id: planContent
            x: 12
            y: 12
            width: Math.max(0, planScroll.availableWidth - 24)
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 560
                Layout.minimumWidth: 420
                spacing: 12

                WaypointList {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 180
                }
                MissionActions {
                    Layout.fillWidth: true
                    Layout.minimumHeight: implicitHeight
                    Layout.preferredHeight: implicitHeight
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 420
                Layout.minimumWidth: 340
                spacing: 12

                WaypointEditor {
                    Layout.fillWidth: true
                    Layout.minimumHeight: implicitHeight
                    Layout.preferredHeight: implicitHeight
                }
                ValidationPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 180
                }
            }
        }
    }
}
