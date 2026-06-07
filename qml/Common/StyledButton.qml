import QtQuick
import QtQuick.Controls.Basic
import LabGCS

Button {
    id: control

    property string variant: "secondary"

    implicitHeight: Theme.controlHeight
    horizontalPadding: 14
    verticalPadding: 8
    font.family: Theme.fontFamily
    font.pixelSize: 13
    font.weight: Font.Medium

    function baseColor() {
        if (variant === "primary") return Theme.accent
        if (variant === "danger") return Theme.dangerSurface
        if (variant === "ghost") return "transparent"
        return Theme.surfaceElevated
    }

    function hoverColor() {
        if (variant === "primary") return Theme.accentHover
        if (variant === "danger") return "#472020"
        if (variant === "ghost") return Theme.hoverSurface
        return Theme.hoverSurface
    }

    function pressedColor() {
        if (variant === "primary") return Theme.accentPressed
        if (variant === "danger") return "#5a2424"
        if (variant === "ghost") return Theme.activeSurface
        return Theme.activeSurface
    }

    function textColor() {
        if (!enabled) return Theme.textDisabled
        if (variant === "primary") return "white"
        if (variant === "danger") return "#ffb4b4"
        return Theme.textPrimary
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.textColor()
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        opacity: control.enabled ? 1.0 : 0.7
    }

    background: Rectangle {
        radius: Theme.radiusSm
        color: !control.enabled ? Theme.disabledSurface
              : control.down ? control.pressedColor()
              : control.hovered ? control.hoverColor()
              : control.baseColor()
        border.width: 1
        border.color: control.activeFocus ? Theme.borderFocus
                    : control.variant === "primary" ? Theme.accent
                    : control.variant === "danger" ? Theme.dangerBorder
                    : Theme.border
        opacity: control.enabled ? 1.0 : 0.58

        Behavior on color { ColorAnimation { duration: 100 } }
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }
}
