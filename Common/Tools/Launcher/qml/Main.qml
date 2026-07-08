import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Window
import SAGELauncher

ApplicationWindow {
    id: window
    width: 1180
    height: 760
    minimumWidth: 960
    minimumHeight: 600
    visible: true
    title: qsTr("SAGE Launcher")
    color: Theme.background
    flags: Qt.Window | Qt.FramelessWindowHint

    InstallationFinder { id: finder }
    SteamCmdManager { id: steamCmd }
    GameLauncher { id: gameLauncher }
    GameCatalogModel { id: catalogModel }

    property string selectedGameId: ""
    property string selectedDisplayName: ""
    property string selectedCoverArt: ""
    property var selectedInstallation: null

    function updateSelectedInstallation() {
        selectedInstallation = selectedGameId.length > 0 ? finder.model.findByGameId(selectedGameId) : null
    }

    function launchSelectedGame() {
        launchError = ""

        const baseGameId = catalogModel.requiresBaseGameId(selectedGameId)
        let baseInstallPath = ""
        if (baseGameId.length > 0) {
            const baseInstallation = finder.model.findByGameId(baseGameId)
            if (!baseInstallation) {
                launchError = qsTr("%1 requires %2 to be installed first.")
                        .arg(selectedDisplayName)
                        .arg(catalogModel.displayNameForId(baseGameId))
                return
            }
            baseInstallPath = baseInstallation.installPath
        }

        gameLauncher.launch(selectedGameId,
                selectedInstallation.installPath,
                selectedInstallation.executablePath,
                LauncherConfig.windowedMode,
                baseInstallPath)
    }

    function resetSelectedInstallation() {
        LauncherConfig.resetInstallation(selectedGameId, selectedInstallation.installPath)
        finder.refresh()
    }

    property bool advancedSettingsExpanded: false
    property string launchError: ""

    onSelectedGameIdChanged: {
        updateSelectedInstallation()
        launchError = ""
    }

    Connections {
        target: finder
        function onRefreshed() { updateSelectedInstallation() }
    }

    Connections {
        target: steamCmd
        function onInstallFinished(success, dir) { if (success) finder.refresh() }
    }

    Connections {
        target: gameLauncher
        function onLaunchFailed(executablePath, reason) { window.launchError = reason }
    }

    SteamLoginDialog {
        id: steamLoginDialog
        steamCmd: steamCmd
        gameDisplayName: window.selectedDisplayName
        appId: LauncherConfig.steamAppId(window.selectedGameId)
        installDir: LauncherConfig.installDirFor(window.selectedGameId)
    }

    AppSettingsDialog {
        id: appSettingsDialog
    }

    FolderDialog {
        id: addFolderDialog
        title: qsTr("Select existing game folder")
        onAccepted: {
            LauncherConfig.addManualInstallDir(window.selectedGameId, selectedFolder)
            finder.refresh()
        }
    }

    Component.onCompleted: {
        finder.refresh()
        if (selectedGameId === "" && catalogModel.rowCount() > 0) {
            const first = catalogModel.get(0)
            selectedGameId = first.gameId
            selectedDisplayName = first.displayName
            selectedCoverArt = first.coverArt
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TitleBar {
            Layout.fillWidth: true
            appWindow: window
            title: window.title
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // --- Sidebar: library list -------------------------------------
            Rectangle {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                color: Theme.panel

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Label {
                        text: qsTr("LIBRARY")
                        color: Theme.subtleText
                        font.bold: true
                        font.pixelSize: 12
                    }

                    ListView {
                        id: libraryList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 6
                        model: catalogModel

                        delegate: GameCard {
                            width: libraryList.width
                            selected: gameId === window.selectedGameId
                            installation: finder.model.findByGameId(gameId)

                            Connections {
                                target: finder
                                function onRefreshed() {
                                    installation = finder.model.findByGameId(gameId)
                                }
                            }

                            onClicked: {
                                window.selectedGameId = gameId
                                window.selectedDisplayName = displayName
                                window.selectedCoverArt = coverArt
                            }
                        }
                    }

                    AppButton {
                        Layout.fillWidth: true
                        enabled: !finder.scanning
                        text: finder.scanning ? qsTr("Scanning...") : qsTr("Rescan installations")
                        iconSource: "qrc:/qt/qml/SAGELauncher/assets/icons/refresh.svg"
                        spinning: finder.scanning
                        onClicked: finder.refresh()
                    }

                    AppButton {
                        Layout.fillWidth: true
                        text: qsTr("⚙ Settings")
                        onClicked: appSettingsDialog.open()
                    }
                }
            }

            // --- Detail panel -------------------------------------------------
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Image {
                    id: heroImage
                    anchors.fill: parent
                    source: window.selectedCoverArt
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    visible: source.toString().length > 0
                }

                Rectangle {
                    anchors.fill: parent
                    visible: heroImage.visible
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#00000000" }
                        GradientStop { position: 0.55; color: "#aa0a0d13" }
                        GradientStop { position: 1.0; color: Theme.background }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: Theme.background
                    visible: !heroImage.visible
                }

                ColumnLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 32
                    spacing: 16

                    Label {
                        text: window.selectedDisplayName
                        color: Theme.text
                        font.pixelSize: 32
                        font.bold: true
                        visible: text.length > 0
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // Installed: play button
                    ColumnLayout {
                        visible: window.selectedInstallation !== null
                        spacing: 6

                        RowLayout {
                            spacing: 12

                            AppButton {
                                text: qsTr("PLAY")
                                iconSource: "qrc:/qt/qml/SAGELauncher/assets/icons/play.svg"
                                primary: true
                                onClicked: window.launchSelectedGame()
                            }

                            Label {
                                text: window.selectedInstallation ? window.selectedInstallation.installPath : ""
                                color: Theme.subtleText
                                visible: LauncherConfig.showInstallPaths
                            }

                            Item { Layout.fillWidth: true }

                            CheckBox {
                                text: qsTr("Windowed")
                                checked: LauncherConfig.windowedMode
                                // onClicked (not onToggled!) -- AbstractButton's setChecked()
                                // emits toggled() on any value change, including the initial
                                // binding evaluation above, not just real user interaction.
                                onClicked: LauncherConfig.windowedMode = checked
                            }
                        }

                        Label {
                            text: window.launchError
                            color: Theme.danger
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            visible: text.length > 0
                        }
                    }

                    // Not installed: SteamCMD / browse panel
                    ColumnLayout {
                        visible: window.selectedInstallation === null && window.selectedGameId !== ""
                        spacing: 10

                        Label {
                            text: qsTr("Not installed.")
                            color: Theme.subtleText
                        }

                        RowLayout {
                            spacing: 10

                            Button {
                                id: installViaSteamButton
                                visible: catalogModel.hasSteamRelease(window.selectedGameId)
                                onClicked: steamLoginDialog.open()

                                contentItem: RowLayout {
                                    spacing: 8
                                    implicitHeight: 20
                                    Image {
                                        source: "qrc:/qt/qml/SAGELauncher/assets/icons/steam-icon.svg"
                                        sourceSize: Qt.size(16, 16)
                                        Layout.alignment: Qt.AlignVCenter
                                    }
                                    Label {
                                        text: qsTr("Install via Steam")
                                        color: "#c7d5e0"
                                        Layout.alignment: Qt.AlignVCenter
                                    }
                                }

                                background: Rectangle {
                                    radius: Theme.radiusSmall
                                    implicitHeight: 36
                                    color: installViaSteamButton.hovered ? "#2a475e" : "#171a21"
                                    border.color: "#66c0f4"
                                    border.width: 1
                                }
                            }

                            AppButton {
                                text: qsTr("Browse for existing installation...")
                                onClicked: addFolderDialog.open()
                            }
                        }
                    }

                    // --- Settings (Steam App ID override, reset, path visibility) ---
                    Label {
                        visible: window.selectedGameId !== ""
                        text: window.advancedSettingsExpanded ? qsTr("▾ Settings") : qsTr("▸ Settings")
                        color: Theme.subtleText
                        font.pixelSize: 11

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: window.advancedSettingsExpanded = !window.advancedSettingsExpanded
                        }
                    }

                    ColumnLayout {
                        visible: window.advancedSettingsExpanded && window.selectedGameId !== ""
                        spacing: 10

                        RowLayout {
                            spacing: 10
                            visible: catalogModel.hasSteamRelease(window.selectedGameId)

                            Label { text: qsTr("Steam App ID:"); color: Theme.text }
                            TextField {
                                id: appIdField
                                Layout.preferredWidth: 140
                                text: window.selectedGameId.length > 0 ? LauncherConfig.steamAppId(window.selectedGameId) : ""
                                onEditingFinished: {
                                    if (window.selectedGameId === "generals")
                                        LauncherConfig.generalsSteamAppId = text
                                    else if (window.selectedGameId === "zerohour")
                                        LauncherConfig.zeroHourSteamAppId = text
                                }
                            }
                        }

                        CheckBox {
                            text: qsTr("Show installation path")
                            checked: LauncherConfig.showInstallPaths
                            onClicked: LauncherConfig.showInstallPaths = checked
                        }

                        AppButton {
                            text: qsTr("Reset installation")
                            visible: window.selectedInstallation !== null
                            onClicked: window.resetSelectedInstallation()
                        }
                    }
                }
            }
        }
    }

    // --- Frameless window resize handles -----------------------------------
    Item {
        anchors.fill: parent
        z: 1000

        MouseArea {
            width: 6
            cursorShape: Qt.SizeHorCursor
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            onPressed: window.startSystemResize(Qt.LeftEdge)
        }
        MouseArea {
            width: 6
            cursorShape: Qt.SizeHorCursor
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            onPressed: window.startSystemResize(Qt.RightEdge)
        }
        MouseArea {
            height: 6
            cursorShape: Qt.SizeVerCursor
            anchors { top: parent.top; left: parent.left; right: parent.right }
            onPressed: window.startSystemResize(Qt.TopEdge)
        }
        MouseArea {
            height: 6
            cursorShape: Qt.SizeVerCursor
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            onPressed: window.startSystemResize(Qt.BottomEdge)
        }
        MouseArea {
            width: 10; height: 10
            cursorShape: Qt.SizeFDiagCursor
            anchors { left: parent.left; top: parent.top }
            onPressed: window.startSystemResize(Qt.LeftEdge | Qt.TopEdge)
        }
        MouseArea {
            width: 10; height: 10
            cursorShape: Qt.SizeBDiagCursor
            anchors { right: parent.right; top: parent.top }
            onPressed: window.startSystemResize(Qt.RightEdge | Qt.TopEdge)
        }
        MouseArea {
            width: 10; height: 10
            cursorShape: Qt.SizeBDiagCursor
            anchors { left: parent.left; bottom: parent.bottom }
            onPressed: window.startSystemResize(Qt.LeftEdge | Qt.BottomEdge)
        }
        MouseArea {
            width: 10; height: 10
            cursorShape: Qt.SizeFDiagCursor
            anchors { right: parent.right; bottom: parent.bottom }
            onPressed: window.startSystemResize(Qt.RightEdge | Qt.BottomEdge)
        }
    }
}
