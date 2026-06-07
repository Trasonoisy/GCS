import QtQuick
import QtQuick.Controls.Basic
import LabGCS

TextField {
    id: control

    implicitHeight: Theme.controlHeight
    leftPadding: 12
    rightPadding: 12
    selectByMouse: true
    color: enabled ? Theme.textPrimary : Theme.textDisabled
    placeholderTextColor: Theme.textMuted
    selectionColor: Theme.accent
    selectedTextColor: "white"
    font.family: Theme.fontFamily
    font.pixelSize: 13

    background: Rectangle {
        radius: Theme.radiusSm
        color: control.enabled ? Theme.inputBackground : Theme.disabledSurface
        border.width: 1
        border.color: control.activeFocus ? Theme.borderFocus : Theme.border
        opacity: control.enabled ? 1.0 : 0.6

        Behavior on border.color { ColorAnimation { duration: 100 } }
    }
}
