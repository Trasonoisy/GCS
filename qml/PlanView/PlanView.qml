import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: "#121212"

    GridLayout {
        anchors.fill: parent
        anchors.margins: 12
        columns: 2
        columnSpacing: 12
        rowSpacing: 12

        WaypointList {
            Layout.column: 0; Layout.row: 0
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.preferredWidth: 480
        }
        WaypointEditor {
            Layout.column: 1; Layout.row: 0
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.preferredWidth: 320
        }
        MissionActions {
            Layout.column: 0; Layout.row: 1
            Layout.fillWidth: true
            Layout.preferredHeight: 260
        }
        ValidationPanel {
            Layout.column: 1; Layout.row: 1
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.preferredHeight: 260
        }
    }
}
