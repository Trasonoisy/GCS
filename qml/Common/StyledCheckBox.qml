import QtQuick
import QtQuick.Controls.Basic
import LabGCS

CheckBox {
    id: control

    spacing: 8
    font.family: Theme.fontFamily
    font.pixelSize: 13

    indicator: Rectangle {
        implicitWidth: 22
        implicitHeight: 22
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 6
        color: control.checked ? Theme.accent : Theme.inputBackground
        border.width: 1
        border.color: control.activeFocus ? Theme.borderFocus
                    : control.checked ? Theme.accent : Theme.border
        opacity: control.enabled ? 1.0 : 0.55

        Text {
            anchors.centerIn: parent
            text: control.checked ? "✓" : ""
            color: "white"
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? Theme.textPrimary : Theme.textDisabled
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        elide: Text.ElideRight
    }
}
