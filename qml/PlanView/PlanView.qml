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

    GridLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapMd
        columns: 2
        columnSpacing: Theme.gapMd
        rowSpacing: Theme.gapMd

        PlanMapPanel {
            Layout.row: 0
            Layout.column: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 760
            Layout.minimumWidth: 520
            Layout.minimumHeight: 320
            Layout.preferredHeight: 360
        }

        MissionActions {
            Layout.row: 0
            Layout.column: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 500
            Layout.minimumWidth: 380
            Layout.minimumHeight: 320
            Layout.preferredHeight: 360
        }

        WaypointList {
            Layout.row: 1
            Layout.column: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 760
            Layout.minimumWidth: 520
            Layout.minimumHeight: 250
            Layout.preferredHeight: 330
            onEditRequested: (index) => waypointEditDialog.openForIndex(index)
        }

        ValidationPanel {
            Layout.row: 1
            Layout.column: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 500
            Layout.minimumWidth: 380
            Layout.minimumHeight: 250
            Layout.preferredHeight: 330
        }
    }
}
