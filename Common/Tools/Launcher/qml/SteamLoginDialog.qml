import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SAGELauncher

Dialog {
    id: dialog

    required property var steamCmd
    required property string gameDisplayName
    required property string appId
    required property string installDir

    property string pendingUsername: ""
    property string pendingPassword: ""
    property bool consoleExpanded: false
    // True from the moment a code is submitted until this attempt's SteamCMD
    // process exits (looping back to steamGuardRequired for a wrong-code
    // retry, or moving on). Gives the "Submit code" click visible feedback.
    property bool verifyingCode: false

    modal: true
    anchors.centerIn: Overlay.overlay
    width: 420
    closePolicy: steamCmd.busy ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)

    // Without this, the body falls back to Qt Quick Controls' default light
    // Popup background -- invisible against it is exactly the bug that hit
    // the Steam button earlier: dark theme text/buttons need a dark body.
    background: Rectangle {
        color: Theme.panel
        border.color: Theme.border
        border.width: 1
    }

    // Steam brand colors (dark navy header, light-blue accent), matching the
    // "Sign in through Steam" button treatment Valve documents for
    // third-party integrations.
    header: Rectangle {
        implicitHeight: 52
        color: "#171a21"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            spacing: 10

            Image {
                source: "qrc:/qt/qml/SAGELauncher/assets/icons/steam-wordmark.svg"
                sourceSize: Qt.size(96, 29)
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    onOpened: {
        pendingUsername = ""
        pendingPassword = ""
        verifyingCode = false
        usernameField.text = ""
        passwordField.text = ""
        guardField.text = ""
    }

    function startSignIn() {
        pendingUsername = usernameField.text
        pendingPassword = passwordField.text
        if (!steamCmd.bootstrapped)
            steamCmd.bootstrap()
        else
            steamCmd.installOrUpdate(dialog.appId, dialog.installDir, pendingUsername, pendingPassword)
    }

    function submitGuardCode() {
        steamCmd.submitSteamGuardCode(guardField.text)
        verifyingCode = true
        guardField.text = ""
    }

    Connections {
        target: dialog.steamCmd
        function onSteamGuardRequiredChanged() {
            if (steamCmd.steamGuardRequired)
                dialog.verifyingCode = false // SteamCMD asked again (e.g. wrong code) -- let the user retry
        }
        function onBootstrapFinished(success) {
            if (success)
                steamCmd.installOrUpdate(dialog.appId, dialog.installDir, dialog.pendingUsername, dialog.pendingPassword)
        }
        function onInstallFinished(success) {
            dialog.verifyingCode = false
            if (success)
                dialog.close()
        }
    }

    contentItem: ColumnLayout {
        spacing: 14

        Label {
            text: qsTr("Downloading %1 via SteamCMD.").arg(dialog.gameDisplayName)
            color: Theme.subtleText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        // --- Credentials step -------------------------------------------
        ColumnLayout {
            visible: !steamCmd.steamGuardRequired
            spacing: 8
            Layout.fillWidth: true

            TextField {
                id: usernameField
                Layout.fillWidth: true
                placeholderText: qsTr("Steam username")
                enabled: !steamCmd.busy
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: qsTr("Steam password")
                echoMode: TextInput.Password
                enabled: !steamCmd.busy
                Keys.onReturnPressed: if (signInButton.enabled) dialog.startSignIn()
            }
        }

        // --- Steam Guard step ---------------------------------------------
        ColumnLayout {
            visible: steamCmd.steamGuardRequired
            spacing: 8
            Layout.fillWidth: true

            Label {
                text: qsTr("Steam Guard code required. If you use email-based Steam Guard, check your inbox. If you use the Mobile Authenticator app, open Steam on your phone and type the current code shown there. SteamCMD has no QR-scan or push-approval option -- the code must be entered below.")
                color: Theme.subtleText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: guardField
                Layout.fillWidth: true
                placeholderText: dialog.verifyingCode ? qsTr("Verifying...") : qsTr("Steam Guard code")
                enabled: !dialog.verifyingCode
                Keys.onReturnPressed: if (signInButton.enabled) dialog.submitGuardCode()
            }
        }

        Label {
            text: steamCmd.statusText
            color: Theme.subtleText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            visible: text.length > 0
        }

        ProgressBar {
            Layout.fillWidth: true
            visible: steamCmd.busy && steamCmd.progress >= 0
            from: 0
            to: 100
            value: steamCmd.progress
        }

        // --- Full SteamCMD output (collapsed by default) -------------------
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: dialog.consoleExpanded ? qsTr("▾ SteamCMD output") : qsTr("▸ SteamCMD output")
                color: Theme.subtleText
                font.pixelSize: 11

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: dialog.consoleExpanded = !dialog.consoleExpanded
                }
            }

            ScrollView {
                visible: dialog.consoleExpanded
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                clip: true

                TextArea {
                    id: consoleOutputArea
                    readOnly: true
                    text: steamCmd.consoleOutput
                    font.family: "monospace"
                    font.pixelSize: 11
                    wrapMode: TextArea.NoWrap
                    color: Theme.text
                    background: Rectangle { color: "#05070b" }
                    onTextChanged: cursorPosition = text.length
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Item { Layout.fillWidth: true }

            AppButton {
                text: qsTr("Cancel")
                onClicked: {
                    steamCmd.cancel()
                    dialog.close()
                }
            }

            Button {
                id: signInButton
                enabled: steamCmd.steamGuardRequired
                        ? (!dialog.verifyingCode && guardField.text.length > 0)
                        : (!steamCmd.busy && usernameField.text.length > 0 && passwordField.text.length > 0)
                onClicked: {
                    if (steamCmd.steamGuardRequired)
                        dialog.submitGuardCode()
                    else
                        dialog.startSignIn()
                }

                contentItem: Label {
                    text: steamCmd.steamGuardRequired
                            ? (dialog.verifyingCode ? qsTr("Verifying...") : qsTr("Submit code"))
                            : (steamCmd.busy ? qsTr("Working...") : qsTr("Sign in & Install"))
                    color: signInButton.enabled ? "#c7d5e0" : "#5c6773"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: !signInButton.enabled ? "#22262d" : (signInButton.hovered ? "#2a475e" : "#171a21")
                    border.color: "#66c0f4"
                    border.width: signInButton.enabled ? 1 : 0
                }
            }
        }
    }
}
