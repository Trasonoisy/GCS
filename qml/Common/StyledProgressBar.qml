import QtQuick
import QtQuick.Controls.Basic
import LabGCS

ProgressBar {
    id: control

    implicitHeight: 8

    background: Rectangle {
        radius: 4
        color: Theme.inputBackground
        border.color: Theme.borderSoft
        border.width: 1
    }

    contentItem: Item {
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: 4
            color: Theme.accent
        }
    }
}
