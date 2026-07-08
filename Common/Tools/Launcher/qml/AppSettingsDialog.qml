import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SAGELauncher

// Application-wide preferences (not tied to whichever game happens to be
// selected) -- currently just display language, but the natural home for
// anything else app-scoped that shows up later.
Dialog {
    id: dialog

    modal: true
    anchors.centerIn: Overlay.overlay
    width: 360
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Without this, both the header and body fall back to Qt Quick Controls'
    // default light Popup style -- invisible against it is exactly the bug
    // that hit the Steam button earlier (see SteamLoginDialog.qml): dark
    // theme text/buttons need a dark body and header.
    background: Rectangle {
        color: Theme.panel
        border.color: Theme.border
        border.width: 1
    }

    header: Rectangle {
        implicitHeight: 40
        color: Theme.panelAlt

        Label {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 14
            text: qsTr("Settings")
            color: Theme.text
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("Language:"); color: Theme.text }
            ComboBox {
                id: languageCombo
                Layout.fillWidth: true
                readonly property var codes: ["system", "en", "de"]
                model: [qsTr("System Default"), qsTr("English"), qsTr("Deutsch")]
                currentIndex: codes.indexOf(LauncherConfig.language)
                onActivated: LauncherConfig.language = codes[currentIndex]
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }
    }
}
