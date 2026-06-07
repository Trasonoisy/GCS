import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Link & Power")

    function statusColor(s) {
        if (s === "Connected") return Theme.success
        if (s === "Stale") return Theme.warning
        return Theme.danger
    }

    function batteryColor(p) {
        if (p < 0) return Theme.textSecondary
        if (p < 25) return Theme.danger
        if (p < 50) return Theme.warning
        return Theme.success
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

        Label { text: qsTr("Link status"); color: Theme.textSecondary }
        Label {
            text: vehicleVm.linkStatusText
            color: root.statusColor(vehicleVm.linkStatusText)
            font.bold: true
        }

        Label { text: qsTr("Heartbeat"); color: Theme.textSecondary }
        Label {
            text: root.heartbeatText(vehicleVm.heartbeatAgeMs)
            color: vehicleVm.heartbeatAgeMs > 2500 ? Theme.warning : "white"
            font.bold: vehicleVm.heartbeatAgeMs > 2500
        }

        Label { text: qsTr("Battery"); color: Theme.textSecondary }
        Label {
            text: vehicleVm.batteryPercent < 0
                  ? qsTr("unknown")
                  : vehicleVm.batteryPercent.toFixed(0) + " %"
            color: root.batteryColor(vehicleVm.batteryPercent)
            font.bold: true
        }

        Label { text: qsTr("Voltage"); color: Theme.textSecondary }
        Label { text: vehicleVm.batteryVoltage.toFixed(2) + " V"; color: "white" }

        Label { text: qsTr("GPS fix type"); color: Theme.textSecondary }
        Label { text: vehicleVm.gpsFixType; color: "white" }

        Label { text: qsTr("Satellites"); color: Theme.textSecondary }
        Label { text: vehicleVm.satellitesVisible; color: "white" }
    }
}
