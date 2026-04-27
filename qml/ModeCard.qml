import QtQuick

PanelSurface {
    id: root

    property string title: ""
    property string subtitle: ""
    property string caption: ""
    property bool selected: false
    property color accentColor: "#7ab88f"
    signal clicked
    signal accepted

    width: parent ? parent.width : 240
    height: 78
    radius: 10
    tint: root.selected ? "#202321" : "#161719"

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        width: 6
        height: 6
        radius: 3
        color: root.selected ? root.accentColor : "#303234"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 14
        anchors.rightMargin: 28
        spacing: 3

        Text {
            text: root.title
            color: "#f1f1ed"
            font.pixelSize: 15
            font.weight: Font.Medium
        }

        Text {
            text: root.subtitle
            color: "#afb1b4"
            font.pixelSize: 11
        }

        Text {
            text: root.caption
            color: "#7b7e82"
            font.pixelSize: 10
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        onDoubleClicked: root.accepted()
    }
}
