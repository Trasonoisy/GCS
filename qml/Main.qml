import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Layouts
import LabGCS

// Top-level navigation: WelcomeView -> (optional) ConnectView -> MainShell.
// Backend wiring is untouched — all view models stay context-properties on
// the QQmlApplicationEngine. Each screen is a self-contained component and
// communicates with this shell only through signals (startMock,
// startConnect, continueToMain, goHome, goConnect).
ApplicationWindow {
    id: window
    width: 1180
    height: 760
    visible: true
    title: qsTr("Lab GCS MVP - Simulation, SITL, Hardware Read-Only")
    color: Theme.appBackground
    font.family: Theme.fontFamily

    palette.window: Theme.appBackground
    palette.windowText: Theme.textPrimary
    palette.base: Theme.inputBackground
    palette.alternateBase: Theme.surfaceRaised
    palette.text: Theme.textPrimary
    palette.button: Theme.surfaceElevated
    palette.buttonText: Theme.textPrimary
    palette.highlight: Theme.accent
    palette.highlightedText: "white"

    StackView {
        id: nav
        anchors.fill: parent
        initialItem: welcomePage

        // Reusable component handles. Defining the pages as Component
        // factories (instead of inline items) lets the StackView own the
        // lifetimes and re-create a fresh page on each push — which is what
        // we want for the Connect screen so its ConnectionPanel re-reads
        // current linkVm state on every visit.
        Component {
            id: welcomePage
            WelcomeView {
                onStartMock:    nav.push(mainPage)
                onStartConnect: nav.push(connectPage)
            }
        }

        Component {
            id: connectPage
            ConnectView {
                onBack:             nav.pop()
                onContinueToMain:   nav.push(mainPage)
            }
        }

        Component {
            id: mainPage
            MainShell {
                onGoHome: {
                    // Drop everything above the Welcome page rather than
                    // pushing a new instance so we do not leak shells.
                    while (nav.depth > 1) nav.pop()
                }
                onGoConnect: {
                    // Pop back to Welcome and push Connect on top, so Back
                    // from Connect still returns to Welcome (not to a stale
                    // Main shell).
                    while (nav.depth > 1) nav.pop()
                    nav.push(connectPage)
                }
            }
        }
    }
}
