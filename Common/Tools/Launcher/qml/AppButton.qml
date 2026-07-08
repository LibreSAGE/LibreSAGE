import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SAGELauncher

// Shared button look for the whole app (sidebar, detail panel, dialogs) so
// nothing falls back to Qt Quick Controls' default light Basic style, which
// clashes with the app's dark theme. Steam-branded actions intentionally
// don't use this -- see SteamLoginDialog.qml -- since Steam's own button
// treatment is a deliberate exception, not an oversight.
Button {
    id: control

    property bool primary: false
    property string iconSource: ""
    // Spins the icon (used for the "rescan" refresh affordance).
    property bool spinning: false

    implicitHeight: 36

    contentItem: RowLayout {
        spacing: 8

        Image {
            id: icon
            visible: control.iconSource.length > 0
            source: control.iconSource
            sourceSize: Qt.size(16, 16)
            Layout.alignment: Qt.AlignVCenter

            RotationAnimator on rotation {
                running: control.spinning
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 900
            }
        }

        Label {
            text: control.text
            color: control.primary ? "#0b0f16" : Theme.text
            font.bold: control.primary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.fillWidth: true
        }
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        implicitHeight: 36
        color: control.primary
                ? (control.hovered ? Qt.lighter(Theme.accent, 1.15) : Theme.accent)
                : (control.hovered ? Theme.panelAlt : "transparent")
        border.color: control.primary ? Theme.accent : Theme.border
        border.width: 1
        opacity: control.enabled ? 1.0 : 0.5
    }
}
