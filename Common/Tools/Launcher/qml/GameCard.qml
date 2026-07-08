import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SAGELauncher

Item {
    id: card

    required property string gameId
    required property string displayName
    required property string coverArt
    property var installation: null
    property bool selected: false

    signal clicked()

    implicitHeight: 76

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusSmall
        color: card.selected ? Theme.panelAlt : "transparent"
        border.color: card.selected ? Theme.accent : "transparent"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 10

            Image {
                Layout.preferredWidth: 46
                Layout.preferredHeight: 60
                source: card.coverArt
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                smooth: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: card.displayName
                    color: Theme.text
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: card.installation ? qsTr("Installed • %1").arg(card.installation.source) : qsTr("Not installed")
                    color: card.installation ? Theme.success : Theme.subtleText
                    font.pixelSize: 11
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: card.clicked()
        }
    }
}
