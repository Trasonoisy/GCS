import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

// Welcome screen, ChatGPT-style: dark uniform background, centered narrow
// column, soft rounded cards instead of buttons, generous whitespace, no
// hard borders. Backend wiring is unchanged — the mock vehicle is already
// running by the time this screen appears (see src/App/main.cpp).
Rectangle {
    id: root
    color: "#212121"

    signal startMock()
    signal startConnect()

    // Cap content width so the layout stays composed when maximised.
    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter:   parent.verticalCenter
            width: Math.min(parent.width - 48, 720)
            spacing: 30

            // ---- avatar + greeting --------------------------------------
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 16

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 56; height: 56; radius: 28
                    color: "#2f2f2f"
                    border.color: "#3f3f46"
                    border.width: 1
                    Label {
                        anchors.centerIn: parent
                        text: "LG"
                        color: "#ECECF1"
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Lab GCS")
                    color: "#ECECF1"
                    font.pixelSize: 30
                    font.weight: Font.Medium
                }
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("How would you like to start?")
                    color: "#8e8ea0"
                    font.pixelSize: 15
                }
            }

            // ---- two suggestion cards -----------------------------------
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 14

                // Card 1 — Mock vehicle
                Rectangle {
                    id: mockCard
                    Layout.preferredWidth: 280
                    Layout.preferredHeight: 160
                    radius: 14
                    color: mockMA.containsMouse ? "#363636" : "#2f2f2f"
                    border.color: "#3f3f46"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 120 } }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        RowLayout {
                            spacing: 8
                            Rectangle { width: 8; height: 8; radius: 4; color: "#FF8A00" }
                            Label {
                                text: qsTr("Mock")
                                color: "#FF8A00"
                                font.pixelSize: 11
                                font.weight: Font.DemiBold
                            }
                        }

                        Label {
                            text: qsTr("Use the simulated vehicle")
                            color: "#ECECF1"
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }
                        Label {
                            text: qsTr("Skip the connection step and fly the built-in mock without opening any real link.")
                            color: "#8e8ea0"
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            wrapMode: Text.WordWrap
                        }
                    }

                    MouseArea {
                        id: mockMA
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.startMock()
                    }
                }

                // Card 2 — Connect real link
                Rectangle {
                    id: connectCard
                    Layout.preferredWidth: 280
                    Layout.preferredHeight: 160
                    radius: 14
                    color: connMA.containsMouse ? "#363636" : "#2f2f2f"
                    border.color: "#3f3f46"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 120 } }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        RowLayout {
                            spacing: 8
                            Rectangle { width: 8; height: 8; radius: 4; color: "#1F6FEB" }
                            Label {
                                text: qsTr("SITL or hardware")
                                color: "#7FB7E0"
                                font.pixelSize: 11
                                font.weight: Font.DemiBold
                            }
                        }

                        Label {
                            text: qsTr("Connect to a real link")
                            color: "#ECECF1"
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }
                        Label {
                            text: qsTr("Open a UDP listener for SITL or a serial port for read-only hardware telemetry.")
                            color: "#8e8ea0"
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            wrapMode: Text.WordWrap
                        }
                    }

                    MouseArea {
                        id: connMA
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.startConnect()
                    }
                }
            }

            // ---- subtle safety footer -----------------------------------
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 580
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: qsTr("This research GCS cannot arm, take off, land, RTL, override RC, or send real manual control.")
                    color: "#6e6e80"
                    font.pixelSize: 11
                }
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Mission transfer is allowed for Mock and SITL only.")
                    color: "#6e6e80"
                    font.pixelSize: 11
                }
            }
        }

        // ---- corner attribution -----------------------------------------
        Label {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 12
            text: qsTr("Map tiles . OpenStreetMap contributors")
            color: "#565669"
            font.pixelSize: 10
        }
    }
}
