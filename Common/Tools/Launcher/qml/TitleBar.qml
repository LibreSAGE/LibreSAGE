import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import SAGELauncher

Item {
    id: titleBar

    required property var appWindow
    property string title: ""

    implicitHeight: 40

    component WindowButton: Rectangle {
        id: btn
        implicitWidth: 46
        implicitHeight: titleBar.height
        property bool danger: false
        property string glyph: ""
        signal clicked()

        color: mouseArea.containsMouse ? (danger ? Theme.danger : Theme.panelAlt) : "transparent"

        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            anchors.centerIn: parent
            text: btn.glyph
            color: Theme.text
            font.pixelSize: 13
            font.family: "sans-serif"
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.panel
    }

    // Drag-to-move: sits underneath the RowLayout below, so it only catches
    // clicks on the empty title bar area, not the buttons on top of it.
    MouseArea {
        anchors.fill: parent
        onPressed: (mouse) => {
            if (mouse.button === Qt.LeftButton)
                titleBar.appWindow.startSystemMove()
        }
        onDoubleClicked: {
            titleBar.appWindow.visibility = (titleBar.appWindow.visibility === Window.Maximized)
                    ? Window.Windowed : Window.Maximized
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Label {
            Layout.leftMargin: 14
            Layout.fillWidth: true
            text: titleBar.title
            color: Theme.subtleText
            font.pixelSize: 12
            font.bold: true
        }

        WindowButton {
            glyph: "–"
            onClicked: titleBar.appWindow.showMinimized()
        }
        WindowButton {
            glyph: titleBar.appWindow.visibility === Window.Maximized ? "❒" : "□"
            onClicked: titleBar.appWindow.visibility = (titleBar.appWindow.visibility === Window.Maximized)
                    ? Window.Windowed : Window.Maximized
        }
        WindowButton {
            glyph: "✕"
            danger: true
            onClicked: titleBar.appWindow.close()
        }
    }
}
