import QtQuick

PanelSurface {
    id: root

    property alias text: label.text
    property bool selected: false
    property color accentColor: "#7ab88f"
    signal clicked

    width: 92
    height: 34
    radius: 8
    tint: root.selected ? "#242926" : "#161719"

    Text {
        id: label
        anchors.centerIn: parent
        color: root.selected ? "#edf3ec" : "#afb1b4"
        font.pixelSize: 12
        font.weight: Font.Medium
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 6
        width: 18
        height: 2
        radius: 1
        color: root.selected ? root.accentColor : "#00000000"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
