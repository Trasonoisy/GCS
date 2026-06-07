import QtQuick
import QtQuick.Controls.Basic
import LabGCS

Slider {
    id: control

    implicitHeight: 34

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 5
        radius: 3
        color: Theme.borderSoft

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: control.enabled ? Theme.accent : Theme.textDisabled
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: 24
        height: 24
        radius: 12
        color: Theme.surfaceElevated
        border.width: 1
        border.color: control.hovered || control.activeFocus ? Theme.borderFocus : Theme.textSecondary

        Behavior on border.color { ColorAnimation { duration: 100 } }
    }
}
