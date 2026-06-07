import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

InfoCard {
    id: root
    title: qsTr("Selected Waypoint")

    property var item: missionVm.selectedItem
    readonly property bool hasItem: missionVm.selectedIndex >= 0

    function update(field, value) {
        if (!hasItem) return
        missionVm.updateWaypointField(missionVm.selectedIndex, field, value)
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8

        Label {
            visible: !root.hasItem
            text: qsTr("Select a waypoint to edit it. TAKEOFF, LAND, RTL, and LOITER here are mission item types, not immediate commands.")
            color: Theme.warning
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 12
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 6
            enabled: root.hasItem
            opacity: enabled ? 1.0 : 0.45

            Label { text: qsTr("Command"); color: Theme.textSecondary }
            StyledComboBox {
                Layout.fillWidth: true
                model: [
                    { text: "WAYPOINT", value: 16 },
                    { text: "TAKEOFF",  value: 22 },
                    { text: "LAND",     value: 21 },
                    { text: "RTL",      value: 20 },
                    { text: "LOITER",   value: 17 },
                ]
                textRole: "text"
                valueRole: "value"
                currentIndex: {
                    if (!root.hasItem) return 0
                    for (var i = 0; i < model.length; ++i)
                        if (model[i].value === item.command) return i
                    return 0
                }
                onActivated: (idx) => root.update("command", model[idx].value)
            }

            Label { text: qsTr("Frame"); color: Theme.textSecondary }
            Label { text: root.hasItem ? item.frameName : ""; color: "white"; wrapMode: Text.WrapAnywhere }

            Label { text: qsTr("Latitude (deg)"); color: Theme.textSecondary }
            StyledTextField {
                Layout.fillWidth: true
                text: root.hasItem ? Number(item.latitudeDeg).toFixed(7) : ""
                validator: DoubleValidator { bottom: -90; top: 90; decimals: 7 }
                onEditingFinished: if (root.hasItem) root.update("latitudeDeg", parseFloat(text))
            }

            Label { text: qsTr("Longitude (deg)"); color: Theme.textSecondary }
            StyledTextField {
                Layout.fillWidth: true
                text: root.hasItem ? Number(item.longitudeDeg).toFixed(7) : ""
                validator: DoubleValidator { bottom: -180; top: 180; decimals: 7 }
                onEditingFinished: if (root.hasItem) root.update("longitudeDeg", parseFloat(text))
            }

            Label { text: qsTr("Altitude (m)"); color: Theme.textSecondary }
            StyledTextField {
                Layout.fillWidth: true
                text: root.hasItem ? Number(item.altitudeM).toFixed(2) : ""
                validator: DoubleValidator { bottom: -500; top: 10000; decimals: 2 }
                onEditingFinished: if (root.hasItem) root.update("altitudeM", parseFloat(text))
            }

            Label { text: qsTr("Hold time (s)"); color: Theme.textSecondary }
            StyledTextField {
                Layout.fillWidth: true
                text: root.hasItem ? Number(item.holdTimeSec).toFixed(1) : ""
                validator: DoubleValidator { bottom: 0; top: 3600; decimals: 1 }
                onEditingFinished: if (root.hasItem) root.update("holdTimeSec", parseFloat(text))
            }

            Label { text: qsTr("Accept radius (m)"); color: Theme.textSecondary }
            StyledTextField {
                Layout.fillWidth: true
                text: root.hasItem ? Number(item.acceptanceRadiusM).toFixed(2) : ""
                validator: DoubleValidator { bottom: 0; top: 500; decimals: 2 }
                onEditingFinished: if (root.hasItem) root.update("acceptanceRadiusM", parseFloat(text))
            }

            Label { text: qsTr("Yaw (deg)"); color: Theme.textSecondary }
            StyledTextField {
                Layout.fillWidth: true
                placeholderText: qsTr("(unchanged)")
                text: {
                    if (!root.hasItem) return ""
                    return (item.yawDeg === undefined || item.yawDeg === null)
                           ? "" : Number(item.yawDeg).toFixed(1)
                }
                onEditingFinished: {
                    if (!root.hasItem) return
                    if (text === "") root.update("yawDeg", undefined)
                    else root.update("yawDeg", parseFloat(text))
                }
            }

            Label { text: qsTr("Auto-continue"); color: Theme.textSecondary }
            StyledCheckBox {
                checked: root.hasItem ? item.autocontinue : true
                onToggled: if (root.hasItem) root.update("autocontinue", checked)
            }
        }
    }
}
