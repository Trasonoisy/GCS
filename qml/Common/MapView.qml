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
    // Emitted when the operator manually pans, pinches, or wheel-zooms the
    // map. Parents that auto-centre on telemetry (FlyMapPanel) should use
    // this to turn off follow-mode so the map stays where the user put it.
    signal userInteracted()

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
        copyrightsVisible: true

        // Pan: Qt 6.11 QtLocation removed the legacy `gesture` property, so
        // map panning is no longer automatic — we drive it from an explicit
        // DragHandler that converts pixel deltas into a centre-coordinate
        // shift. Two finger pinch is handled by PinchHandler below.
        property point _dragRefPx
        property var   _dragRefCoord

        DragHandler {
            id: panHandler
            target: null
            acceptedButtons: Qt.LeftButton
            onActiveChanged: {
                if (active) {
                    mapImpl._dragRefPx    = centroid.position
                    mapImpl._dragRefCoord = mapImpl.toCoordinate(centroid.position)
                    root.userInteracted()
                }
            }
            onCentroidChanged: {
                if (!active || !mapImpl._dragRefCoord) return
                // Re-anchor: take the original world coordinate that was under
                // the finger at press, ask the map where the finger is now in
                // pixels, then shift centre so the same world point stays
                // glued to the new pixel.
                const refPx     = mapImpl.fromCoordinate(mapImpl._dragRefCoord, false)
                const dx        = centroid.position.x - refPx.x
                const dy        = centroid.position.y - refPx.y
                const centrePx  = mapImpl.fromCoordinate(mapImpl.center, false)
                mapImpl.center  = mapImpl.toCoordinate(
                    Qt.point(centrePx.x - dx, centrePx.y - dy))
            }
        }

        PinchHandler {
            target: null
            onActiveChanged: if (active) root.userInteracted()
            onActiveScaleChanged: {
                const z = mapImpl.zoomLevel + Math.log2(activeScale)
                mapImpl.zoomLevel = Math.max(mapImpl.minimumZoomLevel,
                                  Math.min(mapImpl.maximumZoomLevel, z))
            }
        }

        // Wheel zoom (Qt 6 doesn't wire mouse-wheel to Map by default).
        WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: function(event) {
                const delta = event.angleDelta.y / 120
                mapImpl.zoomLevel = Math.max(mapImpl.minimumZoomLevel,
                                  Math.min(mapImpl.maximumZoomLevel,
                                           mapImpl.zoomLevel + delta))
                root.userInteracted()
            }
        }

        // Forward tap events without blocking pan. gesturePolicy:DragThreshold
        // (the default) releases the touch as soon as the pointer moves past
        // the drag threshold, so a click-and-drag becomes a pan, not a tap.
        TapHandler {
            acceptedButtons: Qt.LeftButton
            gesturePolicy: TapHandler.DragThreshold
            onTapped: function(eventPoint) {
                const c = mapImpl.toCoordinate(eventPoint.position)
                root.mapClicked(c)
            }
            onDoubleTapped: function(eventPoint) {
                const c = mapImpl.toCoordinate(eventPoint.position)
                root.mapDoubleClicked(c)
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
