import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtLocation
import QtPositioning
import LabGCS

// Editable map for PlanView. Reads waypoints from MissionViewModel.items,
// draws a numbered marker per waypoint and a polyline connecting them in
// order. Clicking on empty map adds a waypoint at that coordinate (mirrors
// missionVm.addWaypoint() then sets lat/lon). Clicking a marker selects it.
//
// SAFETY: every write goes through MissionViewModel (the only QML-reachable
// mission editor). This panel never touches MAVLink, never bypasses
// SafetyGate — exactly the same as WaypointEditor.qml.
InfoCard {
    id: root
    title: qsTr("Map — flight plan")

    MapView {
        id: planMap
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 320

        // ---- bind to mission view-model ---------------------------------

        // Re-centre once when the first waypoint appears, so the user is
        // not stranded staring at the default Hanoi viewport.
        Connections {
            target: missionVm
            function onItemsChanged() {
                if (missionVm.itemCount === 1) {
                    const it = missionVm.items[0]
                    planMap.setCenter(it.latitudeDeg, it.longitudeDeg)
                }
            }
        }

        // ---- mission polyline -------------------------------------------

        MapPolyline {
            id: missionPath
            line.width: 3
            line.color: Theme.accent
            path: {
                const out = []
                const items = missionVm.items
                for (let i = 0; i < items.length; ++i) {
                    out.push(QtPositioning.coordinate(
                        items[i].latitudeDeg, items[i].longitudeDeg))
                }
                return out
            }
        }

        // ---- waypoint markers (one per mission item) --------------------

        MapItemView {
            model: missionVm.items
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(modelData.latitudeDeg,
                                                     modelData.longitudeDeg)
                anchorPoint.x: marker.width / 2
                anchorPoint.y: marker.height / 2
                sourceItem: Rectangle {
                    id: marker
                    width: 28; height: 28; radius: 14
                    color: (missionVm.selectedIndex === modelData.index)
                           ? Theme.warning : Theme.accent
                    border.color: "white"; border.width: 2
                    Label {
                        anchors.centerIn: parent
                        text: (modelData.seq + 1).toString()
                        color: "white"; font.bold: true; font.pixelSize: 12
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: missionVm.selectedIndex = modelData.index
                    }
                }
            }
        }

        // ---- click empty map to append a waypoint -----------------------

        onMapClicked: function(c) {
            // Avoid spurious adds on tiny mis-clicks: only add if user
            // double-clicks, so clicking a marker (which propagates) selects
            // instead of stacking a new waypoint on top of it.
        }
        onMapDoubleClicked: function(c) {
            missionVm.addWaypoint()
            const newIndex = missionVm.itemCount - 1
            missionVm.updateWaypointField(newIndex, "latitudeDeg",  c.latitude)
            missionVm.updateWaypointField(newIndex, "longitudeDeg", c.longitude)
            missionVm.selectedIndex = newIndex
        }
    }

    // ---- on-map hint ----------------------------------------------------

    Rectangle {
        // Parent to the InfoCard so we sit above the map item.
        parent: planMap
        anchors.bottom: parent.bottom
        anchors.left:   parent.left
        anchors.margins: 8
        color: Theme.surfaceElevated; opacity: 0.85; radius: 4
        border.color: Theme.border
        width: hintLabel.implicitWidth + 16
        height: hintLabel.implicitHeight + 8
        Label {
            id: hintLabel
            anchors.centerIn: parent
            color: Theme.textPrimary
            font.pixelSize: 11
            text: qsTr("Double-click on the map to add a waypoint • click a marker to select")
        }
    }
}
