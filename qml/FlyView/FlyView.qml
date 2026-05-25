import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: "#121212"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            height: 38
            radius: 4
            color: "#181818"
            border.color: "#333333"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 12

                Label {
                    text: qsTr("Active vehicle:")
                    color: "#9A9A9A"
                    font.pixelSize: 12
                }
                Label {
                    text: vehicleVm.vehicleLabel + " / " + vehicleVm.autopilotType
                    color: "white"
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Label {
                    text: vehicleVm.linkStatusText
                    color: vehicleVm.linkStatusText === "Connected" ? "#A0E060"
                          : vehicleVm.linkStatusText === "Stale" ? "#FFC107" : "#FF8080"
                    font.bold: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                Layout.preferredWidth: 390
                Layout.fillHeight: true
                spacing: 12

                TelemetryPanel   { Layout.fillWidth: true }
                LinkStatusPanel  { Layout.fillWidth: true }
                ConnectionPanel  { Layout.fillWidth: true; Layout.fillHeight: true }
            }

            AlertPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
