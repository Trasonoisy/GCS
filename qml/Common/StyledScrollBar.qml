import QtQuick
import QtQuick.Controls.Basic
import LabGCS

ScrollBar {
    id: control

    implicitWidth: 8
    implicitHeight: 8
    padding: 2

    contentItem: Rectangle {
        implicitWidth: 4
        implicitHeight: 4
        radius: 2
        color: control.pressed ? Theme.accent
             : control.hovered ? Theme.textSecondary
             : Theme.textMuted
        opacity: control.active || control.hovered ? 0.75 : 0.35
    }

    background: Rectangle {
        color: "transparent"
    }
}
