import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

Item {
    id: root

    property int waypointIndex: -1
    property int commandValue: 16
    property int frameValue: 6
    property string latitudeText: ""
    property string longitudeText: ""
    property string altitudeText: ""
    property string holdTimeText: ""
    property string acceptRadiusText: ""
    property string yawText: ""
    property bool autoContinueValue: true

    function commandIndex(value) {
        for (var i = 0; i < commandModel.length; ++i) {
            if (commandModel[i].value === value) return i
        }
        return 0
    }

    function frameIndex(value) {
        for (var i = 0; i < frameModel.length; ++i) {
            if (frameModel[i].value === value) return i
        }
        return 3
    }

    function parseOrZero(text) {
        const value = parseFloat(text)
        return isNaN(value) ? 0 : value
    }

    function loadFromItem(index) {
        const item = missionVm.items[index]
        if (item === undefined || item === null) return false

        waypointIndex = index
        commandValue = item.command
        frameValue = item.frame
        latitudeText = Number(item.latitudeDeg).toFixed(7)
        longitudeText = Number(item.longitudeDeg).toFixed(7)
        altitudeText = Number(item.altitudeM).toFixed(2)
        holdTimeText = Number(item.holdTimeSec).toFixed(1)
        acceptRadiusText = Number(item.acceptanceRadiusM).toFixed(2)
        yawText = (item.yawDeg === undefined || item.yawDeg === null
                   || isNaN(Number(item.yawDeg)))
                  ? "" : Number(item.yawDeg).toFixed(1)
        autoContinueValue = item.autocontinue
        return true
    }

    function openForIndex(index) {
        if (!loadFromItem(index)) return
        editorDialog.open()
    }

    function applyChanges() {
        if (waypointIndex < 0 || waypointIndex >= missionVm.itemCount) return

        missionVm.updateWaypointField(waypointIndex, "command", commandValue)
        missionVm.updateWaypointField(waypointIndex, "frame", frameValue)
        missionVm.updateWaypointField(waypointIndex, "latitudeDeg", parseOrZero(latitudeText))
        missionVm.updateWaypointField(waypointIndex, "longitudeDeg", parseOrZero(longitudeText))
        missionVm.updateWaypointField(waypointIndex, "altitudeM", parseOrZero(altitudeText))
        missionVm.updateWaypointField(waypointIndex, "holdTimeSec", parseOrZero(holdTimeText))
        missionVm.updateWaypointField(waypointIndex, "acceptanceRadiusM", parseOrZero(acceptRadiusText))
        missionVm.updateWaypointField(waypointIndex, "yawDeg",
                                      yawText === "" ? undefined : parseFloat(yawText))
        missionVm.updateWaypointField(waypointIndex, "autocontinue", autoContinueValue)
        missionVm.selectedIndex = waypointIndex
        editorDialog.close()
    }

    readonly property var commandModel: [
        { text: "WAYPOINT", value: 16 },
        { text: "TAKEOFF", value: 22 },
        { text: "LAND", value: 21 },
        { text: "RTL", value: 20 },
        { text: "LOITER", value: 17 }
    ]

    readonly property var frameModel: [
        { text: "GLOBAL", value: 0 },
        { text: "GLOBAL_RELATIVE_ALT", value: 3 },
        { text: "GLOBAL_INT", value: 5 },
        { text: "GLOBAL_RELATIVE_ALT_INT", value: 6 },
        { text: "GLOBAL_TERRAIN_ALT", value: 10 }
    ]

    Dialog {
        id: editorDialog
        modal: true
        focus: true
        dim: true
        closePolicy: Popup.CloseOnEscape
        title: qsTr("Edit waypoint")
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        width: Math.min(680, Math.max(360, root.width - 48))
        height: Math.min(620, Math.max(460, root.height - 48))

        background: Rectangle {
            radius: Theme.radiusLg
            color: Theme.surfaceRaised
            border.color: Theme.border
            border.width: 1
        }

        header: Rectangle {
            height: 58
            color: "transparent"
            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.panelPadding
                anchors.rightMargin: Theme.panelPadding
                text: qsTr("Edit waypoint ") + (root.waypointIndex >= 0 ? "#" + (root.waypointIndex + 1) : "")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 18
            }
        }

        contentItem: ColumnLayout {
            spacing: Theme.gapMd

            Label {
                Layout.fillWidth: true
                text: qsTr("TAKEOFF, LAND, RTL, and LOITER here are mission item types, not immediate commands.")
                color: Theme.warning
                wrapMode: Text.Wrap
                font.pixelSize: 12
            }

            ScrollView {
                id: editScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                GridLayout {
                    width: Math.max(0, editScroll.availableWidth)
                    columns: 2
                    columnSpacing: Theme.gapMd
                    rowSpacing: Theme.gapSm

                    Label { text: qsTr("Command"); color: Theme.textSecondary }
                    StyledComboBox {
                        Layout.fillWidth: true
                        model: root.commandModel
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: root.commandIndex(root.commandValue)
                        onActivated: (idx) => root.commandValue = root.commandModel[idx].value
                    }

                    Label { text: qsTr("Frame"); color: Theme.textSecondary }
                    StyledComboBox {
                        Layout.fillWidth: true
                        model: root.frameModel
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: root.frameIndex(root.frameValue)
                        onActivated: (idx) => root.frameValue = root.frameModel[idx].value
                    }

                    Label { text: qsTr("Latitude (deg)"); color: Theme.textSecondary }
                    StyledTextField {
                        id: latField
                        Layout.fillWidth: true
                        text: root.latitudeText
                        validator: DoubleValidator { bottom: -90; top: 90; decimals: 7 }
                        onTextChanged: root.latitudeText = text
                    }

                    Label { text: qsTr("Longitude (deg)"); color: Theme.textSecondary }
                    StyledTextField {
                        id: lonField
                        Layout.fillWidth: true
                        text: root.longitudeText
                        validator: DoubleValidator { bottom: -180; top: 180; decimals: 7 }
                        onTextChanged: root.longitudeText = text
                    }

                    Label { text: qsTr("Altitude (m)"); color: Theme.textSecondary }
                    StyledTextField {
                        id: altField
                        Layout.fillWidth: true
                        text: root.altitudeText
                        validator: DoubleValidator { bottom: -500; top: 10000; decimals: 2 }
                        onTextChanged: root.altitudeText = text
                    }

                    Label { text: qsTr("Hold time (s)"); color: Theme.textSecondary }
                    StyledTextField {
                        id: holdField
                        Layout.fillWidth: true
                        text: root.holdTimeText
                        validator: DoubleValidator { bottom: 0; top: 3600; decimals: 1 }
                        onTextChanged: root.holdTimeText = text
                    }

                    Label { text: qsTr("Accept radius (m)"); color: Theme.textSecondary }
                    StyledTextField {
                        id: radiusField
                        Layout.fillWidth: true
                        text: root.acceptRadiusText
                        validator: DoubleValidator { bottom: 0; top: 500; decimals: 2 }
                        onTextChanged: root.acceptRadiusText = text
                    }

                    Label { text: qsTr("Yaw (deg)"); color: Theme.textSecondary }
                    StyledTextField {
                        id: yawField
                        Layout.fillWidth: true
                        placeholderText: qsTr("(unchanged)")
                        text: root.yawText
                        validator: DoubleValidator { bottom: -360; top: 360; decimals: 1 }
                        onTextChanged: root.yawText = text
                    }

                    Label { text: qsTr("Auto-continue"); color: Theme.textSecondary }
                    StyledCheckBox {
                        checked: root.autoContinueValue
                        onToggled: root.autoContinueValue = checked
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.gapSm

                Item { Layout.fillWidth: true }

                StyledButton {
                    text: qsTr("Cancel")
                    onClicked: editorDialog.close()
                }

                StyledButton {
                    text: qsTr("Apply")
                    variant: "primary"
                    enabled: latField.acceptableInput
                             && lonField.acceptableInput
                             && altField.acceptableInput
                             && holdField.acceptableInput
                             && radiusField.acceptableInput
                             && (yawField.text === "" || yawField.acceptableInput)
                    onClicked: root.applyChanges()
                }
            }
        }
    }
}
