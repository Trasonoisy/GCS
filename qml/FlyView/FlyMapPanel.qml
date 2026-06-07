import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtLocation
import QtPositioning
import LabGCS

// Read-only map for FlyView. Displays:
//   * the planned mission (waypoints + connecting polyline) read from
//     missionVm — kept consistent with what the operator sees in PlanView;
//   * the live vehicle position from vehicleVm, rendered as a heading-
//     aligned arrow marker;
//   * the recent vehicle trail (vehicleVm.trail) as a green polyline.
//
// SAFETY: pure observer. No commands, no SafetyGate touches, nothing here
// can arm/take-off/RTL/disarm. Auto-centring is opt-in via the toggle so
// the operator can pan freely without the map snapping back.
InfoCard {
    id: root
    title: qsTr("Map — flight tracking")

    property bool followVehicle: true
    property bool showTrail: false

    MapView {
        id: flyMap
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 320

        // Any manual pan / pinch / wheel zoom turns off Follow so the map
        // stays where the operator put it. To resume auto-centring, tick
        // the "Follow vehicle" checkbox in the on-map toolbar again.
        onUserInteracted: root.followVehicle = false

        // ---- auto-centre on vehicle when the toggle is on ---------------

        Connections {
            target: vehicleVm
            function onChanged() {
                if (root.followVehicle && vehicleVm.hasValidPosition) {
                    flyMap.setCenter(vehicleVm.latitudeDeg,
                                     vehicleVm.longitudeDeg)
                }
            }
        }

        Component.onCompleted: {
            if (vehicleVm.hasValidPosition) {
                flyMap.setCenter(vehicleVm.latitudeDeg,
                                 vehicleVm.longitudeDeg)
            } else if (missionVm.itemCount > 0) {
                const it = missionVm.items[0]
                flyMap.setCenter(it.latitudeDeg, it.longitudeDeg)
            }
        }

        // ---- planned mission polyline (matches PlanView) ----------------

        MapPolyline {
            line.width: 3
            line.color: Theme.accent
            opacity: 0.85
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

        // ---- planned waypoint markers (smaller, read-only) --------------

        MapItemView {
            model: missionVm.items
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(modelData.latitudeDeg,
                                                     modelData.longitudeDeg)
                anchorPoint.x: dot.width / 2
                anchorPoint.y: dot.height / 2
                sourceItem: Rectangle {
                    id: dot
                    width: 22; height: 22; radius: 11
                    color: Theme.accent; opacity: 0.9
                    border.color: "white"; border.width: 2
                    Label {
                        anchors.centerIn: parent
                        text: (modelData.seq + 1).toString()
                        color: "white"; font.bold: true; font.pixelSize: 10
                    }
                }
            }
        }

        // ---- vehicle trail (green) --------------------------------------

        MapPolyline {
            visible: root.showTrail && vehicleVm.trail.length > 1
            line.width: 2
            line.color: "#7CFC7C"
            opacity: 0.9
            path: {
                const out = []
                const t = vehicleVm.trail
                for (let i = 0; i < t.length; ++i) {
                    out.push(QtPositioning.coordinate(t[i].lat, t[i].lon))
                }
                return out
            }
        }

        // ---- live vehicle marker (heading-aligned triangle) -------------

        MapQuickItem {
            visible: vehicleVm.hasValidPosition
            coordinate: vehicleVm.hasValidPosition
                ? QtPositioning.coordinate(vehicleVm.latitudeDeg,
                                           vehicleVm.longitudeDeg)
                : QtPositioning.coordinate(0, 0)
            anchorPoint.x: vehicleMarker.width  / 2
            anchorPoint.y: vehicleMarker.height / 2
            sourceItem: Item {
                id: vehicleMarker
                width: 38; height: 38
                // Use displayHeadingDeg (course over ground, frozen when
                // stationary) so the marker doesn't spin when the vehicle
                // is parked. The on-map status overlay below still shows
                // the raw autopilot headingDeg for telemetry truthfulness.
                rotation: vehicleVm.displayHeadingDeg
                // Triangle pointing up; rotated by heading. Use text instead
                // of Canvas so telemetry updates cannot trigger requestPaint()
                // on the wrong QML object through a nested sourceItem context.
                Label {
                    anchors.centerIn: parent
                    text: "\u25B2"
                    color: vehicleVm.armed ? Theme.danger : Theme.warning
                    font.bold: true
                    font.pixelSize: 32
                    style: Text.Outline
                    styleColor: "white"
                }
            }
        }
    }

    // ---- on-map toggles / status ---------------------------------------

    Rectangle {
        parent: flyMap
        anchors.bottom: parent.bottom
        anchors.left:   parent.left
        anchors.margins: 8
        color: Theme.surfaceElevated; opacity: 0.9; radius: 4
        border.color: Theme.border
        width: row.implicitWidth + 16
        height: row.implicitHeight + 8

        RowLayout {
            id: row
            anchors.centerIn: parent
            spacing: 10

            StyledCheckBox {
                text: qsTr("Follow vehicle")
                checked: root.followVehicle
                onToggled: root.followVehicle = checked
            }

            StyledCheckBox {
                text: qsTr("Show trail")
                checked: root.showTrail
                onToggled: root.showTrail = checked
            }

            StyledButton {
                text: qsTr("Clear trail")
                onClicked: vehicleVm.clearTrail()
            }

            StyledButton {
                text: qsTr("Fit plan")
                enabled: missionVm.itemCount > 0
                onClicked: {
                    const coords = []
                    const items = missionVm.items
                    for (let i = 0; i < items.length; ++i) {
                        coords.push(QtPositioning.coordinate(
                            items[i].latitudeDeg, items[i].longitudeDeg))
                    }
                    flyMap.fitToCoordinates(coords)
                }
            }
        }
    }

    Rectangle {
        parent: flyMap
        anchors.bottom: parent.bottom
        anchors.right:  parent.right
        anchors.margins: 8
        color: Theme.surfaceElevated; opacity: 0.85; radius: 4
        border.color: Theme.border
        width: posLabel.implicitWidth + 16
        height: posLabel.implicitHeight + 8
        Label {
            id: posLabel
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 11
            text: vehicleVm.hasValidPosition
                  ? qsTr("Drone: %1, %2 • alt %3 m • hdg %4°")
                        .arg(Number(vehicleVm.latitudeDeg).toFixed(5))
                        .arg(Number(vehicleVm.longitudeDeg).toFixed(5))
                        .arg(Number(vehicleVm.relativeAltitudeM).toFixed(1))
                        .arg(Number(vehicleVm.headingDeg).toFixed(0))
                  : qsTr("Drone position: waiting for GPS fix")
        }
    }
}
