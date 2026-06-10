import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: Theme.appBackground

    WaypointEditDialog {
        id: waypointEditDialog
        anchors.fill: parent
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapMd
        spacing: Theme.gapMd

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 760
            Layout.minimumWidth: 520
            spacing: Theme.gapMd

            PlanMapPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 280
                Layout.preferredHeight: 420
            }

            WaypointList {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 210
                Layout.preferredHeight: 300
                onEditRequested: (index) => waypointEditDialog.openForIndex(index)
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 500
            Layout.minimumWidth: 380
            spacing: Theme.gapMd

            MissionActions {
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.preferredHeight: implicitHeight
            }

            ValidationPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 240
            }
        }
    }
}
