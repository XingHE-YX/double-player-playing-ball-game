import QtQuick

Item {
    id: root

    property real radius: 10
    property color tint: "#18191a"
    property color borderColor: "#00000000"
    property real borderWidth: 0

    default property alias content: contentHost.data

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.tint
        border.width: root.borderWidth
        border.color: root.borderColor
    }

    Item {
        id: contentHost
        anchors.fill: parent
    }
}
