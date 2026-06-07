import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: Theme.appBackground

    ScrollView {
        id: planScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        contentHeight: planContent.y + planContent.implicitHeight + 12
        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AsNeeded }
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        RowLayout {
            id: planContent
            x: 12
            y: 12
            width: Math.max(0, planScroll.availableWidth - 24)
            spacing: Theme.gapMd

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 620
                Layout.minimumWidth: 460
                spacing: 12

                PlanMapPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 320
                    Layout.preferredHeight: 420
                }
                WaypointList {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 180
                    Layout.preferredHeight: 220
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
