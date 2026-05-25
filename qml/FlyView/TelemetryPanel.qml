import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Telemetry")

    GridLayout {
        columns: 2
        columnSpacing: 16
        rowSpacing: 6
        Layout.fillWidth: true

        Label { text: qsTr("Vehicle"); color: "#9A9A9A" }
        Label { text: vehicleVm.vehicleLabel; color: "white"; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }

        Label { text: qsTr("Mode source"); color: "#9A9A9A" }
        Label {
            text: vehicleVm.simulated ? qsTr("MockVehicle simulation")
                  : linkVm.hardwareReadOnlyActive ? qsTr("Hardware telemetry")
                  : qsTr("SITL telemetry")
            color: vehicleVm.simulated ? "#FFAA33"
                  : linkVm.hardwareReadOnlyActive ? "#FF8080" : "#7FB7E0"
            font.bold: true
        }

        Label { text: qsTr("Autopilot"); color: "#9A9A9A" }
        Label { text: vehicleVm.autopilotType; color: "white" }

        Label { text: qsTr("System / Comp"); color: "#9A9A9A" }
        Label { text: vehicleVm.systemId + " / " + vehicleVm.componentId; color: "white" }

        Label { text: qsTr("Flight mode"); color: "#9A9A9A" }
        Label { text: vehicleVm.flightMode; color: "white"; font.bold: true }

        Label { text: qsTr("Armed state"); color: "#9A9A9A" }
        Label {
            text: vehicleVm.armed ? qsTr("ARMED - display only") : qsTr("Disarmed")
            color: vehicleVm.armed ? "#FF5252" : "#A0E060"
            font.bold: true
        }

        Label { text: qsTr("Latitude"); color: "#9A9A9A" }
        Label { text: vehicleVm.latitudeDeg.toFixed(6) + " deg"; color: "white" }

        Label { text: qsTr("Longitude"); color: "#9A9A9A" }
        Label { text: vehicleVm.longitudeDeg.toFixed(6) + " deg"; color: "white" }

        Label { text: qsTr("Altitude"); color: "#9A9A9A" }
        Label { text: vehicleVm.relativeAltitudeM.toFixed(1) + " m"; color: "white" }

        Label { text: qsTr("Heading"); color: "#9A9A9A" }
        Label { text: vehicleVm.headingDeg.toFixed(1) + " deg"; color: "white" }

        Label { text: qsTr("Roll / Pitch"); color: "#9A9A9A" }
        Label {
            text: vehicleVm.rollDeg.toFixed(1) + " deg / "
                  + vehicleVm.pitchDeg.toFixed(1) + " deg"
            color: "white"
        }

        Label { text: qsTr("Ground speed"); color: "#9A9A9A" }
        Label { text: vehicleVm.groundSpeedMps.toFixed(2) + " m/s"; color: "white" }
    }
}
