import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Telemetry")

    readonly property int labelSize: 11
    readonly property int valueSize: 11

    ScrollView {
        id: telemetryScroll
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AsNeeded }
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        GridLayout {
            width: Math.max(0, telemetryScroll.availableWidth)
            columns: 2
            columnSpacing: 10
            rowSpacing: 3

            Label {
                text: qsTr("Vehicle")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.vehicleLabel
                color: "white"
                font.bold: true
                font.pixelSize: root.valueSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Mode source")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.simulated ? qsTr("MockVehicle simulation")
                      : linkVm.hardwareReadOnlyActive ? qsTr("Hardware telemetry")
                      : qsTr("SITL telemetry")
                color: vehicleVm.simulated ? Theme.warning
                      : linkVm.hardwareReadOnlyActive ? Theme.danger : Theme.blue
                font.bold: true
                font.pixelSize: root.valueSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Autopilot")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.autopilotType
                color: "white"
                font.pixelSize: root.valueSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("System / Comp")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.systemId + " / " + vehicleVm.componentId
                color: "white"
                font.pixelSize: root.valueSize
            }

            Label {
                text: qsTr("Flight mode")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.flightMode
                color: "white"
                font.bold: true
                font.pixelSize: root.valueSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Armed state")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.armed ? qsTr("ARMED - display only") : qsTr("Disarmed")
                color: vehicleVm.armed ? Theme.danger : Theme.success
                font.bold: true
                font.pixelSize: root.valueSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Latitude")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.latitudeDeg.toFixed(6) + " deg"
                color: "white"
                font.pixelSize: root.valueSize
            }

            Label {
                text: qsTr("Longitude")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.longitudeDeg.toFixed(6) + " deg"
                color: "white"
                font.pixelSize: root.valueSize
            }

            Label {
                text: qsTr("Altitude")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.relativeAltitudeM.toFixed(1) + " m"
                color: "white"
                font.pixelSize: root.valueSize
            }

            Label {
                text: qsTr("Heading")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.headingDeg.toFixed(1) + " deg"
                color: "white"
                font.pixelSize: root.valueSize
            }

            Label {
                text: qsTr("Roll / Pitch")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.rollDeg.toFixed(1) + " deg / "
                      + vehicleVm.pitchDeg.toFixed(1) + " deg"
                color: "white"
                font.pixelSize: root.valueSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Ground speed")
                color: Theme.textSecondary
                font.pixelSize: root.labelSize
            }
            Label {
                text: vehicleVm.groundSpeedMps.toFixed(2) + " m/s"
                color: "white"
                font.pixelSize: root.valueSize
            }
        }
    }
}
