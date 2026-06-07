pragma Singleton

import QtQuick

QtObject {
    readonly property string fontFamily: "Segoe UI"
    readonly property string monoFontFamily: "Cascadia Mono"

    readonly property color appBackground: "#111214"
    readonly property color surface: "#17181a"
    readonly property color surfaceRaised: "#1f2023"
    readonly property color surfaceElevated: "#24262a"
    readonly property color inputBackground: "#0f1012"
    readonly property color hoverSurface: "#2a2c31"
    readonly property color activeSurface: "#34363b"
    readonly property color disabledSurface: "#252629"

    readonly property color border: "#2f3033"
    readonly property color borderSoft: "#26272a"
    readonly property color borderFocus: "#10a37f"

    readonly property color textPrimary: "#ececf1"
    readonly property color textSecondary: "#a8a8b3"
    readonly property color textMuted: "#7d7d89"
    readonly property color textDisabled: "#62626d"

    readonly property color accent: "#10a37f"
    readonly property color accentHover: "#0e8f70"
    readonly property color accentPressed: "#0b765d"
    readonly property color blue: "#5fa8ff"
    readonly property color warning: "#f5a524"
    readonly property color danger: "#ef4444"
    readonly property color success: "#22c55e"

    readonly property color successSurface: "#12321f"
    readonly property color successBorder: "#245f3a"
    readonly property color warningSurface: "#332812"
    readonly property color warningBorder: "#735819"
    readonly property color dangerSurface: "#351818"
    readonly property color dangerBorder: "#7f2d2d"
    readonly property color infoSurface: "#132238"
    readonly property color infoBorder: "#25486f"

    readonly property color listRow: "#17181a"
    readonly property color listRowAlt: "#1b1c1f"
    readonly property color listHover: "#24262a"
    readonly property color listSelected: "#183a31"

    readonly property int radiusSm: 8
    readonly property int radiusMd: 12
    readonly property int radiusLg: 16
    readonly property int controlHeight: 38
    readonly property int panelPadding: 16
    readonly property int gapSm: 8
    readonly property int gapMd: 12
    readonly property int gapLg: 16
}
