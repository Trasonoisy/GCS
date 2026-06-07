import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Rectangle {
    id: root
    color: Theme.appBackground

    function normThrottle() {
        return Math.max(0, Math.min(1, (manualVm.throttle + 1.0) / 2.0))
    }

    function stateColor(name) {
        switch (name) {
            case "Active": return Theme.success
            case "Ready": return Theme.blue
            case "Failsafe":
            case "Blocked": return Theme.danger
            case "WaitingForJoystick": return Theme.warning
            default: return Theme.textSecondary
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
            return Theme.success
        if (manualVm.sinkLabel.indexOf("MANUAL_CONTROL") >= 0)
            return Theme.blue
        if (manualVm.sinkLabel.indexOf("SITL-stub") >= 0)
            return Theme.warning
        return Theme.textSecondary
    }

    ScrollView {
        id: manualScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        contentHeight: manualContent.y + manualContent.implicitHeight + 12
        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AsNeeded }
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: manualContent
            x: 12
            y: 12
            width: Math.max(0, manualScroll.availableWidth - 24)
            spacing: Theme.gapMd

            Rectangle {
                Layout.fillWidth: true
                height: 46
                color: Theme.dangerSurface
                border.color: Theme.dangerBorder
                border.width: 1
                radius: Theme.radiusMd
                Label {
                    anchors.centerIn: parent
                    width: parent.width - 20
                    text: qsTr("Manual control is enabled for MockVehicle and UDP SITL only. Serial hardware remains read-only.")
                    color: "#ffb4b4"
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
                            Label { text: qsTr("State"); color: Theme.textSecondary; Layout.preferredWidth: 110 }
                            Label {
                                text: manualVm.stateName
                                color: root.stateColor(manualVm.stateName)
                                font.bold: true
                                font.pixelSize: 18
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Vehicle"); color: Theme.textSecondary; Layout.preferredWidth: 110 }
                            Label { text: manualVm.vehicleLabel; color: "white"; Layout.fillWidth: true; elide: Text.ElideRight }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Sink"); color: Theme.textSecondary; Layout.preferredWidth: 110 }
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
                            color: Theme.dangerSurface
                            border.color: Theme.dangerBorder
                            Label {
                                id: reasonLabel
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                text: qsTr("Blocked reason: ") + manualVm.blockedReason
                                color: Theme.warning
                                wrapMode: Text.WrapAnywhere
                                font.pixelSize: 12
                            }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                        Label {
                            text: qsTr("Activation checklist")
                            color: Theme.textPrimary
                            font.bold: true
                        }

                        Repeater {
                            model: manualVm.checklist
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: itemText.implicitHeight + 8
                                radius: 3
                                color: modelData.indexOf("[OK]") === 0 ? Theme.successSurface : Theme.dangerSurface
                                border.color: modelData.indexOf("[OK]") === 0 ? Theme.successBorder : Theme.dangerBorder
                                Label {
                                    id: itemText
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    text: modelData
                                    color: modelData.indexOf("[OK]") === 0 ? Theme.success : Theme.danger
                                    font.pixelSize: 12
                                    wrapMode: Text.Wrap
                                }
                            }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            StyledButton {
                                text: qsTr("Enable")
                                variant: "primary"
                                enabled: !manualVm.operatorEnabled
                                onClicked: manualVm.enable()
                            }
                            StyledButton {
                                text: qsTr("Disable")
                                variant: "danger"
                                enabled: manualVm.operatorEnabled
                                onClicked: manualVm.disable()
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            StyledButton {
                                text: manualVm.joystickConnected ? qsTr("Disconnect mock joystick")
                                                                  : qsTr("Connect mock joystick")
                                variant: manualVm.joystickConnected ? "danger" : "primary"
                                onClicked: manualVm.setJoystickConnected(!manualVm.joystickConnected)
                            }
                            StyledButton {
                                text: qsTr("Center axes")
                                onClicked: manualVm.centreAxes()
                            }
                        }

                        Label {
                            text: qsTr("Samples delivered to safe sink: ") + manualVm.totalSamplesSent
                            color: Theme.textSecondary
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

                        Label { text: qsTr("Pitch"); color: Theme.textSecondary }
                        StyledSlider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawPitch; onMoved: manualVm.rawPitch = value }
                        Label { text: qsTr("raw ") + manualVm.rawPitch.toFixed(2) + qsTr(" / proc ") + manualVm.pitch.toFixed(2); color: "white"; font.family: "Consolas" }

                        Label { text: qsTr("Roll"); color: Theme.textSecondary }
                        StyledSlider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawRoll; onMoved: manualVm.rawRoll = value }
                        Label { text: qsTr("raw ") + manualVm.rawRoll.toFixed(2) + qsTr(" / proc ") + manualVm.roll.toFixed(2); color: "white"; font.family: "Consolas" }

                        Label { text: qsTr("Throttle"); color: Theme.textSecondary }
                        StyledSlider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawThrottle; onMoved: manualVm.rawThrottle = value }
                        Label { text: qsTr("raw ") + manualVm.rawThrottle.toFixed(2) + qsTr(" / proc ") + manualVm.throttle.toFixed(2); color: "white"; font.family: "Consolas" }

                        Label { text: qsTr("Yaw"); color: Theme.textSecondary }
                        StyledSlider { Layout.fillWidth: true; from: -1; to: 1; stepSize: 0.01; value: manualVm.rawYaw; onMoved: manualVm.rawYaw = value }
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
                                color: Theme.listRow
                                border.color: manualVm.active ? Theme.successBorder : Theme.border

                                Rectangle {
                                    width: parent.width - 24
                                    height: 1
                                    anchors.centerIn: parent
                                    color: Theme.border
                                }
                                Rectangle {
                                    width: 1
                                    height: parent.height - 24
                                    anchors.centerIn: parent
                                    color: Theme.border
                                }
                                Rectangle {
                                    width: 18
                                    height: 18
                                    radius: 9
                                    color: manualVm.active ? Theme.blue : Theme.textMuted
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
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                }
                            }

                            Label {
                                text: qsTr("stream ") + manualVm.stateName
                                      + qsTr(" / samples ") + manualVm.totalSamplesSent
                                color: manualVm.active ? Theme.success : Theme.textSecondary
                                font.pixelSize: 12
                            }
                        }

                        ColumnLayout {
                            spacing: 12
                            Layout.preferredWidth: 220

                            Label { text: qsTr("Throttle"); color: Theme.textSecondary; font.bold: true }
                            Rectangle {
                                id: throttleTrack
                                Layout.fillWidth: true
                                height: 18
                                radius: 9
                                color: Theme.listRow
                                border.color: Theme.border
                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: parent.width * root.normThrottle()
                                    radius: 9
                                    color: manualVm.active ? Theme.success : Theme.textMuted
                                    Behavior on width { NumberAnimation { duration: 80 } }
                                }
                            }
                            Label {
                                text: qsTr("climb ") + ((manualVm.throttle).toFixed(2))
                                color: "white"
                                font.family: "Consolas"
                            }

                            Label { text: qsTr("Yaw"); color: Theme.textSecondary; font.bold: true }
                            Rectangle {
                                id: yawTrack
                                Layout.fillWidth: true
                                height: 18
                                radius: 9
                                color: Theme.listRow
                                border.color: Theme.border
                                Rectangle {
                                    width: 1
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    color: Theme.border
                                }
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: parent.height
                                    radius: 9
                                    x: manualVm.yaw >= 0
                                       ? parent.width / 2
                                       : parent.width / 2 + manualVm.yaw * parent.width / 2
                                    width: Math.abs(manualVm.yaw) * parent.width / 2
                                    color: manualVm.active ? Theme.warning : Theme.textMuted
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

                            Label { text: qsTr("Altitude"); color: Theme.textSecondary }
                            Label { text: vehicleVm.relativeAltitudeM.toFixed(1) + " m"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Heading"); color: Theme.textSecondary }
                            Label { text: vehicleVm.headingDeg.toFixed(1) + " deg"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Speed"); color: Theme.textSecondary }
                            Label { text: vehicleVm.groundSpeedMps.toFixed(2) + " m/s"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Roll"); color: Theme.textSecondary }
                            Label { text: vehicleVm.rollDeg.toFixed(1) + " deg"; color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Pitch"); color: Theme.textSecondary }
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
                            color: Theme.warning
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            font.pixelSize: 12
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 16
                            rowSpacing: 6
                            Layout.fillWidth: true

                            Label { text: qsTr("x (pitch)"); color: Theme.textSecondary }
                            Label { text: manualVm.lastX + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("y (roll)"); color: Theme.textSecondary }
                            Label { text: manualVm.lastY + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("z (throttle)"); color: Theme.textSecondary }
                            Label { text: manualVm.lastZ + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("r (yaw)"); color: Theme.textSecondary }
                            Label { text: manualVm.lastR + qsTr(" / 1000"); color: "white"; font.family: "Consolas" }

                            Label { text: qsTr("Joystick"); color: Theme.textSecondary }
                            Label {
                                text: manualVm.joystickConnected ? manualVm.joystickName : qsTr("disconnected")
                                color: manualVm.joystickConnected ? Theme.success : Theme.danger
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
