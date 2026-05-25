import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Link & Power")

    function statusColor(s) {
        if (s === "Connected") return "#A0E060"
        if (s === "Stale") return "#FFC107"
        return "#FF5252"
    }

    function batteryColor(p) {
        if (p < 0) return "#9A9A9A"
        if (p < 25) return "#FF5252"
        if (p < 50) return "#FFC107"
        return "#A0E060"
    }

    function heartbeatText(age) {
        if (age < 0) return qsTr("never received")
        if (age > 2500) return (age / 1000.0).toFixed(1) + qsTr(" s ago (stale)")
        return (age / 1000.0).toFixed(1) + qsTr(" s ago")
    }

    GridLayout {
        columns: 2
        columnSpacing: 16
        rowSpacing: 6
        Layout.fillWidth: true

        Label { text: qsTr("Link status"); color: "#9A9A9A" }
        Label {
            text: vehicleVm.linkStatusText
            color: root.statusColor(vehicleVm.linkStatusText)
            font.bold: true
        }

        Label { text: qsTr("Heartbeat"); color: "#9A9A9A" }
        Label {
            text: root.heartbeatText(vehicleVm.heartbeatAgeMs)
            color: vehicleVm.heartbeatAgeMs > 2500 ? "#FFC107" : "white"
            font.bold: vehicleVm.heartbeatAgeMs > 2500
        }

        Label { text: qsTr("Battery"); color: "#9A9A9A" }
        Label {
            text: vehicleVm.batteryPercent < 0
                  ? qsTr("unknown")
                  : vehicleVm.batteryPercent.toFixed(0) + " %"
            color: root.batteryColor(vehicleVm.batteryPercent)
            font.bold: true
        }

        Label { text: qsTr("Voltage"); color: "#9A9A9A" }
        Label { text: vehicleVm.batteryVoltage.toFixed(2) + " V"; color: "white" }

        Label { text: qsTr("GPS fix type"); color: "#9A9A9A" }
        Label { text: vehicleVm.gpsFixType; color: "white" }

        Label { text: qsTr("Satellites"); color: "#9A9A9A" }
        Label { text: vehicleVm.satellitesVisible; color: "white" }
    }
}
