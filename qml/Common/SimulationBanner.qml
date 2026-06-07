import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root
    property bool simulated: false

    visible: simulated
    height: visible ? 30 : 0
    color: Theme.warningSurface
    border.color: Theme.warningBorder
    border.width: visible ? 1 : 0

    Label {
        anchors.centerIn: parent
        width: parent.width - 24
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("SIMULATION MODE - MockVehicle telemetry and mock mission transfer. No real drone is connected.")
        color: Theme.warning
        font.bold: true
        font.family: Theme.fontFamily
        font.pixelSize: 13
        elide: Text.ElideRight
    }
}
