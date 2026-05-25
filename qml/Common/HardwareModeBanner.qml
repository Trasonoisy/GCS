import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root
    property bool active: false

    visible: active
    height: visible ? 34 : 0
    color: "#B71C1C"

    Label {
        anchors.centerIn: parent
        width: parent.width - 24
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("HARDWARE READ-ONLY MODE - serial telemetry only; all command paths are disabled. Remove propellers for bench testing.")
        color: "white"
        font.bold: true
        font.pixelSize: 13
        elide: Text.ElideRight
    }
}
