import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Connection")

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8

        GridLayout {
            columns: 2
            columnSpacing: 12
            rowSpacing: 6
            Layout.fillWidth: true

            Label { text: qsTr("Mode"); color: Theme.textSecondary }
            StyledComboBox {
                Layout.fillWidth: true
                model: linkVm.connectionKindOptions
                currentIndex: linkVm.connectionKind === "Serial" ? 1 : 0
                enabled: !linkVm.connected && !linkVm.serialConnected
                onActivated: (idx) => linkVm.connectionKind = model[idx]
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        ColumnLayout {
            visible: linkVm.connectionKind === "UDP"
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: qsTr("PX4 / ArduPilot SITL telemetry and mission transfer")
                color: Theme.blue
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            GridLayout {
                columns: 2
                columnSpacing: 12
                rowSpacing: 6
                Layout.fillWidth: true

                Label { text: qsTr("Listen host"); color: Theme.textSecondary }
                StyledTextField {
                    Layout.fillWidth: true
                    text: linkVm.listenHost
                    enabled: !linkVm.connected
                    onEditingFinished: linkVm.listenHost = text
                }

                Label { text: qsTr("Listen port"); color: Theme.textSecondary }
                StyledTextField {
                    Layout.fillWidth: true
                    text: linkVm.listenPort
                    enabled: !linkVm.connected
                    validator: IntValidator { bottom: 1; top: 65535 }
                    onEditingFinished: linkVm.listenPort = parseInt(text)
                }

                Label { text: qsTr("Status"); color: Theme.textSecondary }
                Label {
                    text: linkVm.connected
                          ? (linkVm.peerSeen
                             ? qsTr("Connected to peer ") + linkVm.peerHint
                             : qsTr("Listening - waiting for SITL heartbeat"))
                          : qsTr("Disconnected")
                    color: linkVm.connected
                           ? (linkVm.peerSeen ? Theme.success : Theme.warning)
                           : Theme.danger
                    font.bold: true
                    wrapMode: Text.WrapAnywhere
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                StyledButton {
                    text: qsTr("Start UDP listener")
                    variant: "primary"
                    enabled: !linkVm.connected
                    onClicked: linkVm.connectToSitl()
                }
                StyledButton {
                    text: qsTr("Stop")
                    variant: "danger"
                    enabled: linkVm.connected
                    onClicked: linkVm.disconnectFromSitl()
                }
                Item { Layout.fillWidth: true }
            }
        }

        ColumnLayout {
            visible: linkVm.connectionKind === "Serial"
            Layout.fillWidth: true
            spacing: 6

            Rectangle {
                Layout.fillWidth: true
                height: 56
                radius: 4
                color: Theme.dangerSurface
                border.color: Theme.danger
                Label {
                    anchors.centerIn: parent
                    text: qsTr("Hardware Read-Only: telemetry is allowed, outbound commands are refused.")
                    color: "white"
                    font.bold: true
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                    width: parent.width - 16
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Label {
                visible: !linkVm.serialBackendAvailable
                text: qsTr("Qt6::SerialPort is not compiled in. Install Qt SerialPort and reconfigure CMake.")
                color: Theme.warning
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.italic: true
                font.pixelSize: 11
            }

            GridLayout {
                columns: 3
                columnSpacing: 8
                rowSpacing: 6
                Layout.fillWidth: true
                enabled: linkVm.serialBackendAvailable

                Label { text: qsTr("Port"); color: Theme.textSecondary }
                StyledComboBox {
                    Layout.fillWidth: true
                    model: linkVm.availableSerialPorts
                    currentIndex: {
                        for (var i = 0; i < model.length; ++i) {
                            if (model[i] === linkVm.serialPort) return i
                        }
                        return -1
                    }
                    enabled: !linkVm.serialConnected
                    onActivated: (idx) => linkVm.serialPort = model[idx]
                }
                StyledButton {
                    text: qsTr("Refresh")
                    enabled: !linkVm.serialConnected
                    onClicked: linkVm.refreshSerialPorts()
                }

                Label { text: qsTr("Baud"); color: Theme.textSecondary }
                StyledComboBox {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    model: linkVm.serialBaudOptions
                    currentIndex: {
                        for (var i = 0; i < model.length; ++i) {
                            if (parseInt(model[i]) === linkVm.serialBaud) return i
                        }
                        return -1
                    }
                    enabled: !linkVm.serialConnected
                    onActivated: (idx) => linkVm.serialBaud = parseInt(model[idx])
                }

                Label { text: qsTr("Status"); color: Theme.textSecondary }
                Label {
                    Layout.columnSpan: 2
                    text: linkVm.serialConnected
                          ? qsTr("Connected - HARDWARE READ-ONLY")
                          : qsTr("Disconnected")
                    color: linkVm.serialConnected ? Theme.success : Theme.danger
                    font.bold: true
                    wrapMode: Text.WrapAnywhere
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                StyledButton {
                    text: qsTr("Connect serial")
                    variant: "primary"
                    enabled: linkVm.serialBackendAvailable
                             && !linkVm.serialConnected
                             && linkVm.serialPort !== ""
                    onClicked: linkVm.connectSerial()
                }
                StyledButton {
                    text: qsTr("Disconnect")
                    variant: "danger"
                    enabled: linkVm.serialConnected
                    onClicked: linkVm.disconnectSerial()
                }
                Item { Layout.fillWidth: true }
            }
        }

        Label {
            visible: linkVm.lastError.length > 0
            text: qsTr("Connection error: ") + linkVm.lastError
            color: Theme.danger
            wrapMode: Text.WrapAnywhere
            font.pixelSize: 12
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("UDP is for SITL. Serial is read-only hardware telemetry. Arm, takeoff, RTL, mission start, RC override, and real manual control remain disabled.")
            color: Theme.textMuted
            font.italic: true
            font.pixelSize: 11
            wrapMode: Text.WrapAnywhere
            Layout.fillWidth: true
        }
    }
}
