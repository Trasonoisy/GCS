import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

ApplicationWindow {
    id: window
    width: 1180
    height: 760
    visible: true
    title: qsTr("Lab GCS MVP - Simulation, SITL, Hardware Read-Only")

    function modeLabel() {
        if (linkVm.hardwareReadOnlyActive) return qsTr("HARDWARE READ-ONLY")
        if (vehicleVm.simulated) return qsTr("SIMULATION")
        if (missionVm.transferTarget.indexOf("PX4 SITL") === 0) return qsTr("PX4 SITL")
        if (missionVm.transferTarget.indexOf("ArduPilot SITL") === 0) return qsTr("ARDUPILOT SITL")
        if (linkVm.connected) return qsTr("UDP LISTENING")
        return qsTr("NO VEHICLE")
    }

    function modeColor() {
        if (linkVm.hardwareReadOnlyActive) return "#B71C1C"
        if (vehicleVm.simulated) return "#FF8A00"
        if (missionVm.transferAllowed && !vehicleVm.simulated) return "#1F6FEB"
        if (linkVm.connected) return "#7A5A00"
        return "#555555"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        SimulationBanner {
            Layout.fillWidth: true
            simulated: vehicleVm.simulated
        }

        HardwareModeBanner {
            Layout.fillWidth: true
            active: linkVm.hardwareReadOnlyActive
        }

        Rectangle {
            Layout.fillWidth: true
            height: 64
            color: "#1A1A1A"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 16

                ColumnLayout {
                    spacing: 1
                    Layout.preferredWidth: 220
                    Label {
                        text: qsTr("Lab GCS MVP")
                        color: "white"
                        font.bold: true
                        font.pixelSize: 20
                    }
                    Label {
                        text: qsTr("Simulation / SITL / hardware telemetry only")
                        color: "#A8A8A8"
                        font.pixelSize: 11
                    }
                }

                TabBar {
                    id: tabs
                    Layout.fillWidth: true
                    TabButton { text: qsTr("Fly") }
                    TabButton { text: qsTr("Plan") }
                    TabButton { text: qsTr("Manual") }
                }

                Rectangle {
                    Layout.preferredWidth: 170
                    height: 30
                    radius: 4
                    color: window.modeColor()
                    border.color: "#3C3C3C"
                    Label {
                        anchors.centerIn: parent
                        text: window.modeLabel()
                        color: linkVm.hardwareReadOnlyActive || !vehicleVm.simulated ? "white" : "black"
                        font.bold: true
                        font.pixelSize: 12
                    }
                }

                ColumnLayout {
                    spacing: 1
                    Layout.preferredWidth: 230
                    Label {
                        text: qsTr("Commands disabled: arm / takeoff / RTL / mission start")
                        color: "#FFAA33"
                        font.pixelSize: 11
                        wrapMode: Text.Wrap
                    }
                    Label {
                        text: qsTr("Mission transfer: Mock + SITL only")
                        color: "#777777"
                        font.pixelSize: 11
                    }
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            FlyView            { }
            PlanView           { }
            ManualControlPanel { }
        }
    }
}
