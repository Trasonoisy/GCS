import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: "#121212"

    function normThrottle() {
        return Math.max(0, Math.min(1, (manualVm.throttle + 1.0) / 2.0))
    }

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

    function sinkDecoratedLabel() {
        if (manualVm.sinkSimulated)
            return manualVm.sinkLabel + qsTr(" (simulation)")
        if (manualVm.sinkLabel.indexOf("MANUAL_CONTROL") >= 0)
            return manualVm.sinkLabel + qsTr(" (SITL MAVLink)")
        if (manualVm.sinkLabel.indexOf("SITL-stub") >= 0)
            return manualVm.sinkLabel + qsTr(" (stub, no MAVLink sent)")
        return manualVm.sinkLabel
    }

    function sinkColor() {
        if (manualVm.sinkSimulated)
            return "#A0E060"
        if (manualVm.sinkLabel.indexOf("MANUAL_CONTROL") >= 0)
            return "#7FB7E0"
        if (manualVm.sinkLabel.indexOf("SITL-stub") >= 0)
            return "#FFAA33"
        return "#9A9A9A"
    }

    ScrollView {
        id: manualScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        contentHeight: manualContent.y + manualContent.implicitHeight + 12
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: manualContent
            x: 12
            y: 12
            width: Math.max(0, manualScroll.availableWidth - 24)
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
                    text: qsTr("Manual control is enabled for MockVehicle and UDP SITL only. Serial hardware remains read-only.")
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
                    Layout.minimumWidth: 360
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
                                text: root.sinkDecoratedLabel()
                                color: root.sinkColor()
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
                    title: qsTr("Manual Response Monitor")
                    Layout.fillWidth: true
                    Layout.minimumHeight: 260

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 18

                        ColumnLayout {
                            spacing: 8
                            Layout.preferredWidth: 240

                            Rectangle {
                                id: stickScope
                                Layout.preferredWidth: 220
                                Layout.preferredHeight: 180
                                radius: 6
                                color: "#181818"
                                border.color: manualVm.active ? "#2C703A" : "#3C3C3C"

                                Rectangle {
                                    width: parent.width - 24
                                    height: 1
                                    anchors.centerIn: parent
                                    color: "#3C3C3C"
                                }
                                Rectangle {
                                    width: 1
                                    height: parent.height - 24
                                    anchors.centerIn: parent
                                    color: "#3C3C3C"
                                }
                                Rectangle {
                                    width: 18
                                    height: 18
                                    radius: 9
                                    color: manualVm.active ? "#7FB7E0" : "#777777"
                                    border.color: "white"
                                    x: stickScope.width / 2
                                       + manualVm.roll * (stickScope.width / 2 - 18)
                                       - width / 2
                                    y: stickScope.height / 2
                                       - manualVm.pitch * (stickScope.height / 2 - 18)
                                       - height / 2
                                    Behavior on x { NumberAnimation { duration: 80 } }
                                    Behavior on y { NumberAnimation { duration: 80 } }
                                }
                                Label {
                                    anchors.left: parent.left
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 8
                                    text: qsTr("Roll / Pitch")
                                    color: "#9A9A9A"
                                    font.pixelSize: 11
                                }
                            }

                            Label {
                                text: qsTr("stream ") + manualVm.stateName
                                      + qsTr(" / samples ") + manualVm.totalSamplesSent
                                color: manualVm.active ? "#A0E060" : "#9A9A9A"
                                font.pixelSize: 12
                            }
                        }

                        ColumnLayout {
                            spacing: 12
                            Layout.preferredWidth: 220

                            Label { text: qsTr("Throttle"); color: "#9A9A9A"; font.bold: true }
                            Rectangle {
                                id: throttleTrack
                                Layout.fillWidth: true
                                height: 18
                                radius: 9
                                color: "#181818"
                                border.color: "#3C3C3C"
                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: parent.width * root.normThrottle()
                                    radius: 9
                                    color: manualVm.active ? "#A0E060" : "#777777"
                                    Behavior on width { NumberAnimation { duration: 80 } }
                                }
                            }
                            Label {
                                text: qsTr("climb ") + ((manualVm.throttle).toFixed(2))
                                color: "white"
                                font.family: "Consolas"
                            }

                            Label { text: qsTr("Yaw"); color: "#9A9A9A"; font.bold: true }
                            Rectangle {
                                id: yawTrack
                                Layout.fillWidth: true
                                height: 18
                                radius: 9
                                color: "#181818"
                                border.color: "#3C3C3C"
                                Rectangle {
                                    width: 1
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    color: "#555555"
                                }
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: parent.height
                                    radius: 9
                                    x: manualVm.yaw >= 0
                                       ? parent.width / 2
                                       : parent.width / 2 + manualVm.yaw * parent.width / 2
                                    width: Math.abs(manualVm.yaw) * parent.width / 2
                                    color: manualVm.active ? "#FFAA33" : "#777777"
                                    Behavior on x { NumberAnimation { duration: 80 } }
                                    Behavior on width { NumberAnimation { duration: 80 } }
                                }
                            }
                            Label {
                                text: qsTr("turn ") + manualVm.yaw.toFixed(2)
                                color: "white"
                                font.family: "Consolas"
                            }
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 6
                            Layout.fillWidth: true

                            Label { text: qsTr("Altitude"); color: "#9A9A9A" }
                            Label { text: vehicleVm.relativeAltitudeM.toFixed(1) + " m"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Heading"); color: "#9A9A9A" }
                            Label { text: vehicleVm.headingDeg.toFixed(1) + " deg"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Speed"); color: "#9A9A9A" }
                            Label { text: vehicleVm.groundSpeedMps.toFixed(2) + " m/s"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Roll"); color: "#9A9A9A" }
                            Label { text: vehicleVm.rollDeg.toFixed(1) + " deg"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Pitch"); color: "#9A9A9A" }
                            Label { text: vehicleVm.pitchDeg.toFixed(1) + " deg"; color: "white"; font.family: "Consolas" }
                        }
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
                            text: qsTr("These values are sent as MAVLink MANUAL_CONTROL only when the active vehicle is UDP SITL.")
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
}
