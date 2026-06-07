import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root
    property string title: ""
    default property alias content: contentLayout.data

    color: Theme.surfaceRaised
    border.color: Theme.border
    border.width: 1
    radius: Theme.radiusMd

    implicitWidth: 320
    implicitHeight: Math.max(104, cardLayout.implicitHeight + Theme.panelPadding * 2)
    clip: true

    ColumnLayout {
        id: cardLayout
        anchors.fill: parent
        anchors.margins: Theme.panelPadding
        spacing: Theme.gapMd

        Label {
            text: root.title
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 15
            font.family: Theme.fontFamily
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.border
        }

        ColumnLayout {
            id: contentLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.gapSm
        }
    }
}
