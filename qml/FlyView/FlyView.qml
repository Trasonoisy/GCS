import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: "#121212"

    ScrollView {
        id: flyScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        contentHeight: flyContent.y + flyContent.implicitHeight + 12
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: flyContent
            x: 12
            y: 12
            width: Math.max(0, flyScroll.availableWidth - 24)
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
                    Layout.minimumWidth: 360
                    Layout.maximumWidth: 460
                    Layout.fillHeight: true
                    spacing: 12

                    TelemetryPanel   { Layout.fillWidth: true; Layout.minimumHeight: implicitHeight }
                    LinkStatusPanel  { Layout.fillWidth: true; Layout.minimumHeight: implicitHeight }
                    ConnectionPanel  { Layout.fillWidth: true; Layout.minimumHeight: implicitHeight }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 12

                    FlyMapPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 360
                        Layout.preferredHeight: 500
                    }

                    AlertPanel {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 180
                        Layout.preferredHeight: 220
                    }
                }
            }
        }
    }
}
