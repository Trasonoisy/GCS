import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root
    property string title: ""
    default property alias content: contentLayout.data

    color: "#1E1E1E"
    border.color: "#3C3C3C"
    border.width: 1
    radius: 6

    implicitWidth: 320
    implicitHeight: Math.max(96, cardLayout.implicitHeight + 24)
    clip: true

    ColumnLayout {
        id: cardLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: root.title
            color: "#CCCCCC"
            font.bold: true
            font.pixelSize: 14
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#3C3C3C"
        }

        ColumnLayout {
            id: contentLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
        }
    }
}
