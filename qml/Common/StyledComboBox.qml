import QtQuick
import QtQuick.Controls.Basic
import LabGCS

ComboBox {
    id: control

    implicitHeight: Theme.controlHeight
    font.family: Theme.fontFamily
    font.pixelSize: 13

    contentItem: Text {
        leftPadding: 12
        rightPadding: 32
        text: control.displayText
        font: control.font
        color: control.enabled ? Theme.textPrimary : Theme.textDisabled
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: "v"
        color: control.enabled ? Theme.textSecondary : Theme.textDisabled
        font.family: Theme.fontFamily
        font.pixelSize: 13
    }

    background: Rectangle {
        radius: Theme.radiusSm
        color: control.enabled ? Theme.inputBackground : Theme.disabledSurface
        border.width: 1
        border.color: control.activeFocus || control.popup.visible ? Theme.borderFocus : Theme.border
        opacity: control.enabled ? 1.0 : 0.6
    }

    delegate: ItemDelegate {
        width: control.width
        height: 34
        highlighted: control.highlightedIndex === index
        text: control.textRole !== "" && modelData && modelData[control.textRole] !== undefined
              ? modelData[control.textRole]
              : modelData
        font.family: Theme.fontFamily
        font.pixelSize: 13

        contentItem: Text {
            text: parent.text
            font: parent.font
            color: parent.highlighted ? Theme.textPrimary : Theme.textSecondary
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: Theme.radiusSm
            color: parent.highlighted ? Theme.hoverSurface : "transparent"
        }

        onClicked: {
            control.currentIndex = index
            control.popup.close()
            control.activated(index)
        }
    }

    popup: Popup {
        y: control.height + 6
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 260)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            radius: Theme.radiusMd
            color: Theme.surfaceElevated
            border.color: Theme.border
            border.width: 1
        }
    }
}
