import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: Theme.appBackground

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapSm
        spacing: Theme.gapSm

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: Theme.radiusMd
            color: Theme.surfaceRaised
            border.color: Theme.border
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10

                Label {
                    text: qsTr("Active vehicle:")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                }
                Label {
                    text: vehicleVm.vehicleLabel + " / " + vehicleVm.autopilotType
                    color: Theme.textPrimary
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Label {
                    text: vehicleVm.linkStatusText
                    color: vehicleVm.linkStatusText === "Connected" ? Theme.success
                          : vehicleVm.linkStatusText === "Stale" ? Theme.warning : Theme.danger
                    font.bold: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.gapSm

            ColumnLayout {
                Layout.preferredWidth: 370
                Layout.minimumWidth: 300
                Layout.maximumWidth: 420
                Layout.fillHeight: true
                spacing: Theme.gapSm

                TelemetryPanel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 150
                    Layout.preferredHeight: 210
                    Layout.maximumHeight: 230
                }

                LinkStatusPanel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 132
                    Layout.preferredHeight: implicitHeight
                    Layout.maximumHeight: 156
                }

                AlertPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 140
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                FlyMapPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 300
                    Layout.preferredHeight: 520
                }
            }
        }
    }
}
