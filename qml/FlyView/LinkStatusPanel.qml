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
        if (age < 0) return qsTr("never")
        if (age > 2500) return (age / 1000.0).toFixed(1) + qsTr(" s stale")
        return (age / 1000.0).toFixed(1) + qsTr(" s ago")
    }

    component Metric: ColumnLayout {
        property string label: ""
        property string value: ""
        property color valueColor: Theme.textPrimary
        property bool strong: false

        spacing: 2
        Layout.fillWidth: true

        Label {
            Layout.fillWidth: true
            text: label
            color: Theme.textMuted
            font.pixelSize: 10
            elide: Text.ElideRight
        }
        Label {
            Layout.fillWidth: true
            text: value
            color: valueColor
            font.pixelSize: 12
            font.bold: strong
            elide: Text.ElideRight
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: Theme.gapSm
        rowSpacing: 6

        Metric {
            label: qsTr("Link")
            value: vehicleVm.linkStatusText
            valueColor: root.statusColor(vehicleVm.linkStatusText)
            strong: true
        }
        Metric {
            label: qsTr("Battery")
            value: vehicleVm.batteryPercent < 0
                   ? qsTr("unknown")
                   : vehicleVm.batteryPercent.toFixed(0) + " %"
            valueColor: root.batteryColor(vehicleVm.batteryPercent)
            strong: true
        }
        Metric {
            label: qsTr("Heartbeat")
            value: root.heartbeatText(vehicleVm.heartbeatAgeMs)
            valueColor: vehicleVm.heartbeatAgeMs > 2500 ? Theme.warning : Theme.textPrimary
            strong: vehicleVm.heartbeatAgeMs > 2500
        }
        Metric {
            label: qsTr("Voltage")
            value: vehicleVm.batteryVoltage.toFixed(2) + " V"
        }
        Metric {
            label: qsTr("GPS fix")
            value: vehicleVm.gpsFixType.toString()
        }
        Metric {
            label: qsTr("Satellites")
            value: vehicleVm.satellitesVisible.toString()
        }
    }
}
