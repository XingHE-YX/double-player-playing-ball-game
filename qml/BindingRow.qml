import QtQuick

PanelSurface {
    id: root

    property string label: ""
    property string value: ""
    property bool active: false
    signal clicked

    height: 40
    radius: 9
    tint: root.active ? "#202321" : "#161719"

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: "#f1f1ed"
        font.pixelSize: 12
        font.weight: Font.Medium
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 102
        height: 24
        radius: 6
        color: root.active ? "#2b4835" : "#252729"

        Text {
            anchors.centerIn: parent
            text: root.value
            color: root.active ? "#edf3ec" : "#b0b2b5"
            font.pixelSize: 11
            font.weight: Font.Medium
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
