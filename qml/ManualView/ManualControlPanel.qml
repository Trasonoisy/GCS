import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: "#121212"

    function stateColor(name) {
        switch (name) {
            case "Active": return "#A0E060"
            case "Ready": return "#7FB7E0"
            case "Failsafe":
            case "Blocked": return "#FF5252"
            case "WaitingForJoystick": return "#FFC107"
            default: return "#9A9A9A"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            height: 44
            color: "#5A1F1F"
            border.color: "#FF5252"
            radius: 4
            Label {
                anchors.centerIn: parent
                width: parent.width - 20
                text: qsTr("Manual control framework only. MockVehicle consumes samples; SITL uses a logged stub; hardware control is disabled.")
                color: "white"
                font.bold: true
                font.pixelSize: 12
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            InfoCard {
                title: qsTr("Safety Gate")
                Layout.preferredWidth: 390
                Layout.fillHeight: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("State"); color: "#9A9A9A"; Layout.preferredWidth: 110 }
                        Label {
                            text: manualVm.stateName
                            color: root.stateColor(manualVm.stateName)
                            font.bold: true
                            font.pixelSize: 18
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Vehicle"); color: "#9A9A9A"; Layout.preferredWidth: 110 }
                        Label { text: manualVm.vehicleLabel; color: "white"; Layout.fillWidth: true; elide: Text.ElideRight }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Sink"); color: "#9A9A9A"; Layout.preferredWidth: 110 }
                        Label {
                            text: manualVm.sinkLabel
                                  + (manualVm.sinkSimulated ? qsTr(" (simulation)")
                                                            : qsTr(" (stub, no MAVLink sent)"))
                            color: manualVm.sinkSimulated ? "#A0E060" : "#FFAA33"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        visible: manualVm.blockedReason !== ""
                        Layout.fillWidth: true
                        implicitHeight: reasonLabel.implicitHeight + 14
                        radius: 4
                        color: "#3A2424"
                        border.color: "#7A3A3A"
                        Label {
                            id: reasonLabel
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            text: qsTr("Blocked reason: ") + manualVm.blockedReason
                            color: "#FFAA33"
                            wrapMode: Text.WrapAnywhere
                            font.pixelSize: 12
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#3C3C3C" }

                    Label {
                        text: qsTr("Activation checklist")
                        color: "#CCCCCC"
                        font.bold: true
                    }

                    Repeater {
                        model: manualVm.checklist
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: itemText.implicitHeight + 8
                            radius: 3
                            color: modelData.indexOf("[OK]") === 0 ? "#17311C" : "#3A2424"
                            border.color: modelData.indexOf("[OK]") === 0 ? "#2C703A" : "#7A3A3A"
                            Label {
                                id: itemText
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                text: modelData
                                color: modelData.indexOf("[OK]") === 0 ? "#A0E060" : "#FF8080"
                                font.pixelSize: 12
                                wrapMode: Text.Wrap
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#3C3C3C" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Button {
                            text: qsTr("Enable")
                            enabled: !manualVm.operatorEnabled
                            onClicked: manualVm.enable()
                        }
                        Button {
                            text: qsTr("Disable")
                            enabled: manualVm.operatorEnabled
                            onClicked: manualVm.disable()
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Button {
                            text: manualVm.joystickConnected ? qsTr("Disconnect mock joystick")
                                                              : qsTr("Connect mock joystick")
                            onClicked: manualVm.setJoystickConnected(!manualVm.joystickConnected)
                        }
                        Button {
                            text: qsTr("Center axes")
                            onClicked: manualVm.centreAxes()
                        }
                    }

                    Label {
                        text: qsTr("Samples delivered to safe sink: ") + manualVm.totalSamplesSent
                        color: "#9A9A9A"
                        font.pixelSize: 12
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                InfoCard {
                    title: qsTr("Mock Joystick Input")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 3
                        columnSpacing: 12
                        rowSpacing: 10
                        Layout.fillWidth: true

                        Label { text: qsTr("Pitch"); color: "#9A9A9A" }
                        Slider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawPitch; onMoved: manualVm.rawPitch = value }
                        Label { text: qsTr("raw ") + manualVm.rawPitch.toFixed(2) + qsTr(" / proc ") + manualVm.pitch.toFixed(2); color: "white"; font.family: "Consolas" }

                        Label { text: qsTr("Roll"); color: "#9A9A9A" }
                        Slider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawRoll; onMoved: manualVm.rawRoll = value }
                        Label { text: qsTr("raw ") + manualVm.rawRoll.toFixed(2) + qsTr(" / proc ") + manualVm.roll.toFixed(2); color: "white"; font.family: "Consolas" }

                        Label { text: qsTr("Throttle"); color: "#9A9A9A" }
                        Slider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawThrottle; onMoved: manualVm.rawThrottle = value }
                        Label { text: qsTr("raw ") + manualVm.rawThrottle.toFixed(2) + qsTr(" / proc ") + manualVm.throttle.toFixed(2); color: "white"; font.family: "Consolas" }

                        Label { text: qsTr("Yaw"); color: "#9A9A9A" }
                        Slider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawYaw; onMoved: manualVm.rawYaw = value }
                        Label { text: qsTr("raw ") + manualVm.rawYaw.toFixed(2) + qsTr(" / proc ") + manualVm.yaw.toFixed(2); color: "white"; font.family: "Consolas" }
                    }
                }

                InfoCard {
                    title: qsTr("Packed MANUAL_CONTROL Values")
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Display only in this MVP. SITL sink is logged; hardware sink does not exist.")
                            color: "#FFAA33"
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            font.pixelSize: 12
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 16
                            rowSpacing: 6
                            Layout.fillWidth: true

                            Label { text: qsTr("x (pitch)"); color: "#9A9A9A" }
                            Label { text: manualVm.lastX + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("y (roll)"); color: "#9A9A9A" }
                            Label { text: manualVm.lastY + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("z (throttle)"); color: "#9A9A9A" }
                            Label { text: manualVm.lastZ + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("r (yaw)"); color: "#9A9A9A" }
                            Label { text: manualVm.lastR + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Joystick"); color: "#9A9A9A" }
                            Label {
                                text: manualVm.joystickConnected ? manualVm.joystickName : qsTr("disconnected")
                                color: manualVm.joystickConnected ? "#A0E060" : "#FF8080"
                                font.bold: true
                            }
                        }
                    }
                }
            }
        }
    }
}
