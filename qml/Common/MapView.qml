import QtQuick
import QtQuick.Controls.Basic
import QtLocation
import QtPositioning

// Shared base map. Owns the OSM plugin, the Map item, attribution, and
// pan/zoom controls. Parents (FlyMapPanel / PlanMapPanel) drop MapItemView /
// MapPolyline / MapQuickItem children into `mapItems` to render their own
// overlays (waypoints, vehicle marker, trails).
//
// SAFETY / POLICY:
//   * Uses the OpenStreetMap raster tile service via the built-in Qt OSM
//     plugin. The OSM Tile Usage Policy requires:
//       - a clear, contactable User-Agent (the plugin sets one per Qt
//         version, which is acceptable for low-volume lab use);
//       - visible attribution (rendered as the bottom-right overlay);
//       - no aggressive bulk downloads (we use cacheable browsing, no tile
//         pre-fetch loops, no automated panning).
//   * This is a viewer only — it never sends MAVLink. Click handling is
//     surfaced as a signal so PlanMapPanel can call MissionViewModel.
Item {
    id: root

    // ---- public API -----------------------------------------------------

    // Initial centre — Hanoi by default, far from the (0,0) Atlantic null.
    property real initialLatitude:  21.0285
    property real initialLongitude: 105.8542
    property real initialZoom:      15

    // Render children passed by the parent (waypoints, vehicle, polyline,
    // and also non-visual helpers like Connections / Timer). We alias to
    // Map.data — not Map.children — because:
    //   1. `data` accepts QObject subclasses, so Connections/Timer work;
    //   2. MapItem subclasses (MapPolyline, MapQuickItem, MapItemView) are
    //      routed through addMapItem() automatically when added via `data`,
    //      which is the API contract QtLocation expects.
    default property alias mapData: mapImpl.data

    // Forwarded so parents can issue commands like setCenter().
    readonly property alias map: mapImpl

    signal mapClicked(var coordinate)
    signal mapDoubleClicked(var coordinate)

    function setCenter(lat, lon) {
        if (!isFinite(lat) || !isFinite(lon)) return
        if (lat === 0 && lon === 0) return
        mapImpl.center = QtPositioning.coordinate(lat, lon)
    }

    function setZoom(z) {
        mapImpl.zoomLevel = Math.max(mapImpl.minimumZoomLevel,
                                     Math.min(mapImpl.maximumZoomLevel, z))
    }

    function fitToCoordinates(coords) {
        if (!coords || coords.length === 0) return
        if (coords.length === 1) {
            mapImpl.center = coords[0]
            return
        }
        mapImpl.fitViewportToGeoShape(
            QtPositioning.shapeToRectangle(
                QtPositioning.polygon(coords)))
    }

    // ---- plugin ---------------------------------------------------------

    Plugin {
        id: osmPlugin
        name: "osm"
        // Use the standard OSM raster style. `osm.useragent` lets us identify
        // ourselves to the tile server — required by the OSM Tile Usage
        // Policy. Disabling providers repository avoids opportunistic fetches.
        PluginParameter { name: "osm.useragent";              value: "LabGCS/0.1 (lab research GCS)" }
        PluginParameter { name: "osm.mapping.providersrepository.disabled"; value: "true" }
        // Map type "Street Map" (id 1) is the default; we let the user
        // see the attribution rendered by the OSM provider.
    }

    // ---- map ------------------------------------------------------------

    Map {
        id: mapImpl
        anchors.fill: parent
        plugin: osmPlugin
        center: QtPositioning.coordinate(root.initialLatitude, root.initialLongitude)
        zoomLevel: root.initialZoom
        // Smooth zoom on wheel; keeps panning responsive even when the
        // backend Vehicle is updating telemetry at 10+ Hz.
        copyrightsVisible: true

        // Forward click events. We pass the geo-coordinate so PlanView can
        // turn a click into a waypoint without needing to know the pixel
        // geometry.
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            propagateComposedEvents: true
            onClicked: function(mouse) {
                const c = mapImpl.toCoordinate(Qt.point(mouse.x, mouse.y))
                root.mapClicked(c)
                mouse.accepted = false
            }
            onDoubleClicked: function(mouse) {
                const c = mapImpl.toCoordinate(Qt.point(mouse.x, mouse.y))
                root.mapDoubleClicked(c)
                mouse.accepted = false
            }
        }
    }

    // ---- zoom controls --------------------------------------------------

    Column {
        anchors.right: parent.right
        anchors.top:   parent.top
        anchors.rightMargin: 8
        anchors.topMargin:   8
        spacing: 4

        Rectangle {
            width: 32; height: 32; radius: 4
            color: "#222222"; opacity: 0.85
            border.color: "#555555"
            Label {
                anchors.centerIn: parent
                text: "+"; color: "white"; font.bold: true; font.pixelSize: 18
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.setZoom(mapImpl.zoomLevel + 1)
            }
        }
        Rectangle {
            width: 32; height: 32; radius: 4
            color: "#222222"; opacity: 0.85
            border.color: "#555555"
            Label {
                anchors.centerIn: parent
                text: "−"; color: "white"; font.bold: true; font.pixelSize: 18
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.setZoom(mapImpl.zoomLevel - 1)
            }
        }
    }

    // ---- on-map status -------------------------------------------------

    Rectangle {
        anchors.left:   parent.left
        anchors.top:    parent.top
        anchors.margins: 8
        color: "#222222"; opacity: 0.85; radius: 4
        border.color: "#555555"
        width: zoomLabel.implicitWidth + 16
        height: zoomLabel.implicitHeight + 8
        Label {
            id: zoomLabel
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 11
            text: qsTr("Zoom %1 • © OpenStreetMap contributors")
                  .arg(Number(mapImpl.zoomLevel).toFixed(1))
        }
    }
}
