import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root
    property bool simulated: false

    visible: simulated
    height: visible ? 30 : 0
    color: "#FF8A00"

    Label {
        anchors.centerIn: parent
        width: parent.width - 24
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("SIMULATION MODE - MockVehicle telemetry and mock mission transfer. No real drone is connected.")
        color: "black"
        font.bold: true
        font.pixelSize: 13
        elide: Text.ElideRight
    }
}
