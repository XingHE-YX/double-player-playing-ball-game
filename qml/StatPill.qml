import QtQuick

PanelSurface {
    id: root

    property string label: ""
    property int value: 0
    property color accentColor: "#7ab88f"

    width: 132
    height: 68
    radius: 10
    tint: "#161719"

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 4

        Text {
            text: root.label
            color: root.accentColor
            font.pixelSize: 10
            font.weight: Font.Medium
        }

        Text {
            text: root.value
            color: "#f1f1ed"
            font.pixelSize: 21
            font.weight: Font.Medium
        }
    }
}
