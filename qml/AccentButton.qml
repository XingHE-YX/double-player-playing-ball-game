import QtQuick

PanelSurface {
    id: root

    property alias text: label.text
    property color accentColor: "#7ab88f"
    property bool selected: false
    signal clicked

    height: 38
    radius: 10
    tint: buttonArea.pressed ? Qt.darker(accentColor, 1.08) : accentColor
    borderWidth: root.selected ? 2 : 0
    borderColor: "#edf3ec"

    Text {
        id: label
        anchors.centerIn: parent
        color: "#101311"
        font.pixelSize: 13
        font.weight: Font.Medium
    }

    MouseArea {
        id: buttonArea
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
