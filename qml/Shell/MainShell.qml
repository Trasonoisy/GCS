import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

// Main app shell, ChatGPT-style chrome: dark uniform background, soft
// rounded pill tabs, small mode chip, pill action buttons. Tab contents
// (PlanView / FlyView / ManualControlPanel) and view models stay exactly
// as they are — only the surrounding chrome is restyled.
Rectangle {
    id: root
    color: Theme.appBackground

    // Default to Plan so the operator opens straight into the planner.
    property int initialTabIndex: 0
    property int currentTabIndex: initialTabIndex

    signal goHome()
    signal goConnect()

    // ---- mode helpers (unchanged semantics) -----------------------------
    function modeLabel() {
        if (linkVm.hardwareReadOnlyActive) return qsTr("HARDWARE READ-ONLY")
        if (vehicleVm.simulated) return qsTr("SIMULATION")
        if (missionVm.transferTarget.indexOf("PX4 SITL") === 0) return qsTr("PX4 SITL")
        if (missionVm.transferTarget.indexOf("ArduPilot SITL") === 0) return qsTr("ARDUPILOT SITL")
        if (linkVm.connected) return qsTr("UDP LISTENING")
        return qsTr("NO VEHICLE")
    }

    function modeAccent() {
        if (linkVm.hardwareReadOnlyActive) return Theme.danger
        if (vehicleVm.simulated) return Theme.warning
        if (missionVm.transferAllowed && !vehicleVm.simulated) return Theme.accent
        if (linkVm.connected) return Theme.warning
        return Theme.textMuted
    }

    Connections {
        target: missionVm
        function onMissionPreviewStarted() {
            vehicleVm.clearTrail()
            root.currentTabIndex = 1
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Existing banners stay (functional, safety-relevant).
        SimulationBanner {
            Layout.fillWidth: true
            simulated: vehicleVm.simulated
        }
        HardwareModeBanner {
            Layout.fillWidth: true
            active: linkVm.hardwareReadOnlyActive
        }

        // ---- top header strip (ChatGPT-style minimal chrome) ------------
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: Theme.appBackground
            // 1px hairline divider below the header
            Rectangle {
                anchors.left:  parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.borderSoft
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                spacing: 14

                // brand: small avatar circle + name
                RowLayout {
                    spacing: 10
                    Layout.preferredWidth: 180
                    Rectangle {
                        width: 30; height: 30; radius: 15
                        color: Theme.surfaceElevated
                        border.color: Theme.border
                        border.width: 1
                        Label {
                            anchors.centerIn: parent
                            text: "LG"
                            color: Theme.textPrimary
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                        }
                    }
                    Label {
                        text: qsTr("Lab GCS")
                        color: Theme.textPrimary
                        font.pixelSize: 15
                        font.weight: Font.Medium
                    }
                }

                // pill tab bar (centred)
                Item { Layout.fillWidth: true }

                Rectangle {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: tabRow.implicitWidth + 8
                    radius: 18
                    color: Theme.surfaceRaised
                    border.color: Theme.border
                    border.width: 1

                    RowLayout {
                        id: tabRow
                        anchors.fill: parent
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        spacing: 0

                        Repeater {
                            id: tabRepeater
                            model: [ qsTr("Plan"), qsTr("Fly"), qsTr("Manual") ]
                            delegate: Rectangle {
                                Layout.preferredHeight: 28
                                Layout.preferredWidth: tabLbl.implicitWidth + 28
                                Layout.alignment: Qt.AlignVCenter
                                radius: 14
                                color: root.currentTabIndex === index
                                       ? Theme.activeSurface
                                       : (tabMA.containsMouse ? Theme.hoverSurface : "transparent")
                                Behavior on color { ColorAnimation { duration: 100 } }

                                Label {
                                    id: tabLbl
                                    anchors.centerIn: parent
                                    text: modelData
                                    color: root.currentTabIndex === index ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 12
                                    font.weight: root.currentTabIndex === index ? Font.DemiBold : Font.Normal
                                }
                                MouseArea {
                                    id: tabMA
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.currentTabIndex = index
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // mode chip
                Rectangle {
                    Layout.preferredHeight: 28
                    Layout.preferredWidth: modeChipRow.implicitWidth + 18
                    radius: 14
                    color: Theme.surfaceRaised
                    border.color: Theme.border
                    border.width: 1
                    RowLayout {
                        id: modeChipRow
                        anchors.centerIn: parent
                        spacing: 6
                        Rectangle { width: 8; height: 8; radius: 4; color: root.modeAccent() }
                        Label {
                            text: root.modeLabel()
                            color: Theme.textPrimary
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                    }
                }

                // pill action buttons: Change link / Home
                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 110
                    radius: 15
                    color: changeMA.containsMouse ? Theme.hoverSurface : Theme.surfaceElevated
                    border.color: Theme.border
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 100 } }
                    Label {
                        anchors.centerIn: parent
                        text: qsTr("Change link")
                        color: Theme.textPrimary
                        font.pixelSize: 11
                    }
                    MouseArea {
                        id: changeMA
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.goConnect()
                        ToolTip.visible: containsMouse
                        ToolTip.text: qsTr("Open the connection screen again.")
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 80
                    radius: 15
                    color: homeMA.containsMouse ? Theme.hoverSurface : Theme.surfaceElevated
                    border.color: Theme.border
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 100 } }
                    Label {
                        anchors.centerIn: parent
                        text: qsTr("Home")
                        color: Theme.textPrimary
                        font.pixelSize: 11
                    }
                    MouseArea {
                        id: homeMA
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.goHome()
                        ToolTip.visible: containsMouse
                        ToolTip.text: qsTr("Return to the welcome screen. Telemetry keeps running.")
                    }
                }
            }
        }

        // ---- tab content (unchanged) ------------------------------------
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentTabIndex

            // Order matches the pill bar above: Plan / Fly / Manual.
            PlanView           { }
            FlyView            { }
            ManualControlPanel { }
        }
    }
}
