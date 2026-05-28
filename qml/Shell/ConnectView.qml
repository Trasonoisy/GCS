import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

// Connect screen, ChatGPT-style: dark uniform background, narrow centered
// column, soft rounded card hosting the existing ConnectionPanel, pill
// buttons at the bottom. Backend logic is unchanged — we only wrap the
// existing ConnectionPanel and add Back / Continue navigation signals.
Rectangle {
    id: root
    color: "#212121"

    signal back()
    signal continueToMain()

    readonly property bool anyRealLink: linkVm.connected || linkVm.serialConnected

    // ---- top minimal header (no harsh background) -----------------------
    RowLayout {
        id: topBar
        anchors.left:  parent.left
        anchors.right: parent.right
        anchors.top:   parent.top
        anchors.leftMargin:  18
        anchors.rightMargin: 18
        anchors.topMargin:   14
        spacing: 10

        // back chevron, pill-shaped
        Rectangle {
            id: backPill
            Layout.preferredHeight: 32
            Layout.preferredWidth: 80
            radius: 16
            color: backMA.containsMouse ? "#363636" : "#2f2f2f"
            border.color: "#3f3f46"
            Behavior on color { ColorAnimation { duration: 100 } }

            Label {
                anchors.centerIn: parent
                text: qsTr("< Back")
                color: "#ECECF1"
                font.pixelSize: 12
            }
            MouseArea {
                id: backMA
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.back()
            }
        }

        Item { Layout.fillWidth: true }

        // tiny status chip on the right
        Rectangle {
            Layout.preferredHeight: 26
            Layout.preferredWidth: chipLabel.implicitWidth + 22
            radius: 13
            color: root.anyRealLink ? "#1f3b1f" : "#2f2f2f"
            border.color: root.anyRealLink ? "#3f6f3f" : "#3f3f46"
            RowLayout {
                anchors.centerIn: parent
                spacing: 6
                Rectangle {
                    width: 6; height: 6; radius: 3
                    color: root.anyRealLink ? "#10A37F" : "#8e8ea0"
                }
                Label {
                    id: chipLabel
                    text: root.anyRealLink ? qsTr("Link active") : qsTr("No link . will use Mock")
                    color: root.anyRealLink ? "#9be6c2" : "#8e8ea0"
                    font.pixelSize: 11
                }
            }
        }
    }

    // ---- centered content ----------------------------------------------
    ScrollView {
        anchors.top:    topBar.bottom
        anchors.bottom: footer.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.topMargin:    8
        anchors.bottomMargin: 8
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Item {
            width: parent.width

            ColumnLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width - 48, 760)
                spacing: 18

                // ---- greeting block -------------------------------------
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 6
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Connect to a vehicle")
                        color: "#ECECF1"
                        font.pixelSize: 24
                        font.weight: Font.Medium
                    }
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Pick UDP for SITL, or Serial for a real autopilot (telemetry only).")
                        color: "#8e8ea0"
                        font.pixelSize: 13
                    }
                }

                // ---- existing ConnectionPanel, re-skinned ---------------
                // ConnectionPanel inherits from InfoCard (a Rectangle), so we
                // override its colour / border / radius here to match the
                // surrounding ChatGPT-style palette without duplicating any
                // of its logic or layout.
                ConnectionPanel {
                    Layout.fillWidth: true
                    color: "#2a2a2a"
                    border.color: "#3f3f46"
                    radius: 14
                }

                // ---- three mini "how it works" cards --------------------
                GridLayout {
                    Layout.fillWidth: true
                    columns: 3
                    rowSpacing: 10
                    columnSpacing: 10

                    Repeater {
                        model: [
                            { dot: "#1F6FEB", title: qsTr("UDP (SITL)"),
                              body: qsTr("Listens for PX4 or ArduPilot SITL. Mission upload and download enabled.") },
                            { dot: "#FF5252", title: qsTr("Serial (hardware)"),
                              body: qsTr("Read-only. Telemetry shown, but commands and mission writes are refused.") },
                            { dot: "#FF8A00", title: qsTr("Mock vehicle"),
                              body: qsTr("Always running. Continuing without a real link will fly the mock.") }
                        ]
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 96
                            radius: 12
                            color: "#2a2a2a"
                            border.color: "#3f3f46"
                            border.width: 1
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 6
                                RowLayout {
                                    spacing: 6
                                    Rectangle { width: 8; height: 8; radius: 4; color: modelData.dot }
                                    Label {
                                        text: modelData.title
                                        color: "#ECECF1"
                                        font.pixelSize: 12
                                        font.weight: Font.DemiBold
                                    }
                                }
                                Label {
                                    text: modelData.body
                                    color: "#8e8ea0"
                                    font.pixelSize: 11
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- footer with pill Continue button ------------------------------
    Item {
        id: footer
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.bottom: parent.bottom
        height: 72

        Rectangle {
            anchors.left:  parent.left
            anchors.right: parent.right
            anchors.top:   parent.top
            height: 1
            color: "#2f2f2f"
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            spacing: 12

            Label {
                Layout.fillWidth: true
                color: "#8e8ea0"
                font.pixelSize: 12
                text: root.anyRealLink
                      ? qsTr("Connected. You can continue.")
                      : qsTr("No real link opened. Continuing will use the Mock vehicle.")
            }

            Rectangle {
                id: continuePill
                Layout.preferredHeight: 40
                Layout.preferredWidth: 160
                radius: 20
                color: contMA.containsMouse ? "#f5f5f5" : "#ECECF1"
                Behavior on color { ColorAnimation { duration: 100 } }

                Label {
                    anchors.centerIn: parent
                    text: qsTr("Continue >")
                    color: "#1a1a1a"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }
                MouseArea {
                    id: contMA
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.continueToMain()
                }
            }
        }
    }
}
