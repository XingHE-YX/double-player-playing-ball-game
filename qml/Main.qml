pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import GameApp

ApplicationWindow {
    id: window

    width: 980
    height: 720
    minimumWidth: 860
    minimumHeight: 640
    visible: true
    title: "双球对战"
    color: "#101112"
    font.family: Qt.platform.os === "osx" ? "PingFang SC" : (Qt.platform.os === "windows" ? "Segoe UI" : "Noto Sans")

    readonly property real arenaScale: Math.min((window.width - 96) / gameController.gameWidth, (window.height - 112) / gameController.gameHeight)
    readonly property bool startVisible: gameController.state === GameController.Start
    readonly property bool runningVisible: gameController.state === GameController.Running
    readonly property bool pauseVisible: gameController.state === GameController.Pause
    readonly property bool gameOverVisible: gameController.state === GameController.GameOver
    readonly property color accentGreen: "#7ab88f"
    readonly property color accentGreenMuted: "#628f71"
    readonly property color panelColor: "#17181a"
    readonly property color panelRaised: "#1c1d1f"
    readonly property color trackColor: "#252729"
    readonly property color textPrimary: "#f1f1ed"
    readonly property color textSecondary: "#b0b2b5"
    readonly property color textMuted: "#7c8084"

    background: Rectangle {
        color: window.color
    }

    Column {
        visible: !window.startVisible
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 20
        spacing: 4

        Text {
            text: "双球对战"
            color: window.textPrimary
            font.pixelSize: 24
            font.weight: Font.Medium
        }

        Text {
            text: "极简竞技场"
            color: window.textSecondary
            font.pixelSize: 12
        }
    }

    PanelSurface {
        id: statusCard
        visible: !window.startVisible
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 20
        width: 220
        height: 72
        tint: window.panelRaised

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 4

            Text {
                text: window.pauseVisible ? "已暂停" : (window.gameOverVisible ? "已结束" : "进行中")
                color: window.textPrimary
                font.pixelSize: 14
                font.weight: Font.Medium
            }

            Text {
                text: window.runningVisible
                    ? (gameController.mode === GameController.Pve
                        ? "人机难度：" + gameController.aiDifficultyLabel
                        : "别让球越过你的边线。")
                    : "键位和排行榜会自动保存。"
                color: window.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }
    }

    Item {
        id: arenaHost
        visible: !window.startVisible
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 96
        width: gameController.gameWidth * window.arenaScale + 16
        height: gameController.gameHeight * window.arenaScale + 16

        PanelSurface {
            anchors.fill: parent
            radius: 12
            tint: window.panelColor
        }

        Item {
            id: arena
            width: gameController.gameWidth
            height: gameController.gameHeight
            scale: window.arenaScale
            transformOrigin: Item.TopLeft
            x: 8
            y: 8
            clip: true

            Rectangle {
                anchors.fill: parent
                radius: 10
                color: window.panelRaised
            }

            Rectangle {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: 1
                color: window.trackColor
                opacity: 0.55
            }

            Repeater {
                model: 22

                Rectangle {
                    required property int index

                    width: 1
                    height: 10
                    radius: 0.5
                    x: arena.width / 2 - width / 2
                    y: 26 + index * 25
                    color: window.trackColor
                    opacity: index % 2 === 0 ? 0.6 : 0.25
                }
            }

            Rectangle {
                x: 16
                y: 14
                width: 182
                height: 18
                radius: 5
                color: window.trackColor

                Rectangle {
                    x: 0
                    y: 0
                    width: parent.width * gameController.energy1 / 20
                    height: parent.height
                    radius: 5
                    color: window.accentGreen
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: (gameController.mode === GameController.Pve ? gameController.displayPlayerName : "玩家1") + " " + gameController.energy1 + "/20"
                    color: window.textPrimary
                    font.pixelSize: 10
                    font.weight: Font.Medium
                }
            }

            Rectangle {
                anchors.top: parent.top
                anchors.topMargin: 14
                anchors.right: parent.right
                anchors.rightMargin: 16
                width: 182
                height: 18
                radius: 5
                color: window.trackColor

                Rectangle {
                    anchors.right: parent.right
                    width: parent.width * gameController.energy2 / 20
                    height: parent.height
                    radius: 5
                    color: window.accentGreenMuted
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: (gameController.mode === GameController.Pve ? "电脑" : "玩家2") + " " + gameController.energy2 + "/20"
                    color: window.textPrimary
                    font.pixelSize: 10
                    font.weight: Font.Medium
                }
            }

            Rectangle {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 14
                width: gameController.mode === GameController.Pve ? 94 : 54
                height: 18
                radius: 5
                color: window.trackColor

                Text {
                    anchors.centerIn: parent
                    text: gameController.modeLabel
                    color: window.textSecondary
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }
            }

            Rectangle {
                x: gameController.paddle1X
                y: gameController.paddle1Y
                width: gameController.paddleWidth
                height: gameController.paddleHeight
                radius: 4
                color: "#ecece7"
            }

            Rectangle {
                x: gameController.paddle2X
                y: gameController.paddle2Y
                width: gameController.paddleWidth
                height: gameController.paddleHeight
                radius: 4
                color: "#d8dad6"
            }

            Repeater {
                model: gameController.balls

                Item {
                    required property var modelData

                    width: modelData.size * 5.0
                    height: width
                    x: modelData.x - width / 2
                    y: modelData.y - height / 2

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: window.accentGreen
                        opacity: 0.14
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.56
                        height: width
                        radius: width / 2
                        color: "#8fd0a5"
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.2
                        height: width
                        radius: width / 2
                        color: "#f4f7ef"
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 14
                width: 384
                height: 24
                radius: 6
                color: window.trackColor

                Text {
                    anchors.fill: parent
                    anchors.margins: 8
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: gameController.controlsHintText
                    color: window.textSecondary
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: window.startVisible

        Column {
            anchors.centerIn: parent
            width: Math.min(parent.width - 48, 860)
            spacing: 18

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "双球对战"
                color: window.textPrimary
                font.pixelSize: 36
                font.weight: Font.Medium
            }

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: "输入名字后开始。键位、难度和排行榜都会自动保存。"
                color: window.textSecondary
                font.pixelSize: 13
            }

            Row {
                width: parent.width
                height: 516
                spacing: 16

                PanelSurface {
                    id: startPanel
                    width: parent.width * 0.5
                    height: parent.height
                    tint: window.panelRaised

                    ScrollView {
                        id: startScroll
                        anchors.fill: parent
                        anchors.margins: 14
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                        Column {
                            width: Math.max(0, startScroll.availableWidth)
                            spacing: 12

                            Text {
                                text: "开始与设置"
                                color: window.textPrimary
                                font.pixelSize: 17
                                font.weight: Font.Medium
                            }

                            Text {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: gameController.menuSelection === 1
                                    ? "上下切换模式，左右切换难度。点击键位项后按新按键即可替换。"
                                    : "菜单支持方向键和回车。键位设置会在下次启动时自动恢复。"
                                color: window.textSecondary
                                font.pixelSize: 11
                            }

                            Text {
                                text: "玩家名称"
                                color: window.textPrimary
                                font.pixelSize: 12
                                font.weight: Font.Medium
                            }

                            TextField {
                                id: playerNameField
                                width: parent.width
                                height: 38
                                text: gameController.playerName
                                placeholderText: "输入你的名字"
                                maximumLength: 8
                                color: window.textPrimary
                                placeholderTextColor: window.textMuted
                                selectByMouse: true
                                selectionColor: "#2d4738"
                                selectedTextColor: window.textPrimary
                                onTextEdited: gameController.playerName = text
                                onActiveFocusChanged: gameController.setTextInputActive(activeFocus)
                                background: Rectangle {
                                    radius: 8
                                    color: window.trackColor
                                }
                            }

                            ModeCard {
                                title: "双人对战"
                                subtitle: "两名玩家"
                                caption: "共用一套键盘"
                                selected: gameController.menuSelection === 0
                                accentColor: window.accentGreen
                                onClicked: {
                                    playerNameField.focus = false
                                    gameController.selectMenu(0)
                                }
                                onAccepted: gameController.startPvp()
                            }

                            ModeCard {
                                title: "人机对战"
                                subtitle: "挑战电脑"
                                caption: "支持三档难度"
                                selected: gameController.menuSelection === 1
                                accentColor: window.accentGreenMuted
                                onClicked: {
                                    playerNameField.focus = false
                                    gameController.selectMenu(1)
                                }
                                onAccepted: gameController.startPve()
                            }

                            Column {
                                visible: gameController.menuSelection === 1
                                spacing: 8

                                Text {
                                    text: "电脑难度"
                                    color: window.textPrimary
                                    font.pixelSize: 12
                                    font.weight: Font.Medium
                                }

                                Row {
                                    spacing: 8

                                    DifficultyChip {
                                        text: "简单"
                                        selected: gameController.aiDifficulty === GameController.AiEasy
                                        accentColor: window.accentGreenMuted
                                        onClicked: {
                                            playerNameField.focus = false
                                            gameController.selectAiDifficulty(GameController.AiEasy)
                                        }
                                    }

                                    DifficultyChip {
                                        text: "普通"
                                        selected: gameController.aiDifficulty === GameController.AiMedium
                                        accentColor: window.accentGreen
                                        onClicked: {
                                            playerNameField.focus = false
                                            gameController.selectAiDifficulty(GameController.AiMedium)
                                        }
                                    }

                                    DifficultyChip {
                                        text: "困难"
                                        selected: gameController.aiDifficulty === GameController.AiHard
                                        accentColor: "#a7d4b4"
                                        onClicked: {
                                            playerNameField.focus = false
                                            gameController.selectAiDifficulty(GameController.AiHard)
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: gameController.aiDifficulty === GameController.AiEasy
                                        ? "简单：预判和移动都会主动放水，更适合熟悉节奏。"
                                        : (gameController.aiDifficulty === GameController.AiMedium
                                            ? "普通：保留一定预判和速度，接近标准对战强度。"
                                            : "困难：精确预判来球路线，但移动速度与玩家一致。")
                                    color: window.textMuted
                                    font.pixelSize: 11
                                }
                            }

                            Text {
                                text: "键位设置"
                                color: window.textPrimary
                                font.pixelSize: 12
                                font.weight: Font.Medium
                            }

                            Repeater {
                                model: gameController.keyBindings

                                BindingRow {
                                    required property var modelData

                                    width: parent.width
                                    label: modelData.label
                                    value: modelData.action === gameController.rebindingAction ? "按任意键" : modelData.keyLabel
                                    active: modelData.action === gameController.rebindingAction
                                    onClicked: {
                                        playerNameField.focus = false
                                        gameController.beginRebinding(modelData.action)
                                    }
                                }
                            }

                            Text {
                                visible: gameController.rebindingAction !== -1
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: "正在等待新的按键。再次点击当前项可取消。"
                                color: window.textMuted
                                font.pixelSize: 11
                            }

                            AccentButton {
                                width: parent.width
                                text: gameController.menuSelection === 0
                                    ? "开始双人对战"
                                    : ("开始人机对战 · " + gameController.aiDifficultyLabel)
                                accentColor: window.accentGreen
                                onClicked: {
                                    playerNameField.focus = false
                                    gameController.startSelectedMode()
                                }
                            }
                        }
                    }
                }

                PanelSurface {
                    id: leaderboardPanel
                    width: parent.width - startPanel.width - parent.spacing
                    height: parent.height
                    tint: window.panelRaised

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        Row {
                            width: parent.width
                            height: 28

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 72
                                text: "排行榜"
                                color: window.textPrimary
                                font.pixelSize: 17
                                font.weight: Font.Medium
                            }

                            PanelSurface {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 72
                                height: 28
                                radius: 8
                                tint: clearBoardArea.pressed ? "#242729" : "#202224"

                                Text {
                                    anchors.centerIn: parent
                                    text: "清空"
                                    color: gameController.leaderboard.length === 0 ? window.textMuted : window.textSecondary
                                    font.pixelSize: 11
                                    font.weight: Font.Medium
                                }

                                MouseArea {
                                    id: clearBoardArea
                                    anchors.fill: parent
                                    enabled: gameController.leaderboard.length > 0
                                    onClicked: gameController.resetLeaderboard()
                                }
                            }
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "记录人机模式下的接球次数，按成绩从高到低排序。"
                            color: window.textSecondary
                            font.pixelSize: 11
                        }

                        Item {
                            width: parent.width
                            height: leaderboardPanel.height - 86

                            ListView {
                                id: leaderboardList
                                anchors.fill: parent
                                clip: true
                                spacing: 8
                                model: gameController.leaderboard

                                delegate: PanelSurface {
                                    required property var modelData

                                    width: leaderboardList.width
                                    height: 62
                                    radius: 9
                                    tint: index === 0 ? "#1a1c1d" : "#151618"

                                    Rectangle {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 24
                                        height: 24
                                        radius: 6
                                        color: "#252729"
                                    }

                                    Text {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        width: 24
                                        anchors.verticalCenter: parent.verticalCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        text: index + 1
                                        color: index === 0 ? "#7c9587" : "#6a6f73"
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                    }

                                    Item {
                                        id: scoreBlock
                                        anchors.right: parent.right
                                        anchors.rightMargin: 12
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 56
                                        height: 34

                                        Column {
                                            anchors.fill: parent
                                            spacing: 2

                                            Text {
                                                anchors.right: parent.right
                                                text: modelData.score
                                                color: window.textPrimary
                                                font.pixelSize: 18
                                                font.weight: Font.Medium
                                            }

                                            Text {
                                                anchors.right: parent.right
                                                text: "接球"
                                                color: window.textMuted
                                                font.pixelSize: 10
                                            }
                                        }
                                    }

                                    Column {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 42
                                        anchors.right: scoreBlock.left
                                        anchors.rightMargin: 12
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 2

                                        Text {
                                            width: parent.width
                                            text: modelData.name
                                            color: window.textPrimary
                                            font.pixelSize: 13
                                            font.weight: Font.Medium
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            width: parent.width
                                            text: modelData.difficulty + " · " + modelData.result + " · " + modelData.playedAt
                                            color: window.textMuted
                                            font.pixelSize: 10
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }

                            Text {
                                visible: gameController.leaderboard.length === 0
                                anchors.centerIn: parent
                                width: parent.width - 24
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                text: "还没有排行榜记录。开始一局人机对战后，这里会保存你的成绩。"
                                color: window.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "当前键位：" + gameController.controlsHintText
                color: window.textMuted
                font.pixelSize: 11
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: window.pauseVisible

        Rectangle {
            anchors.fill: parent
            color: "#70000000"
        }

        PanelSurface {
            anchors.centerIn: parent
            width: 288
            height: 188
            tint: window.panelRaised

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 10

                Text {
                    text: "已暂停"
                    color: window.textPrimary
                    font.pixelSize: 21
                    font.weight: Font.Medium
                }

                Text {
                    text: "继续当前对局，或直接退出游戏。"
                    color: window.textSecondary
                    font.pixelSize: 12
                }

                AccentButton {
                    width: parent.width
                    text: "继续游戏"
                    accentColor: window.accentGreen
                    selected: gameController.pauseMenuSelection === 0
                    onClicked: gameController.resumeGame()
                }

                AccentButton {
                    width: parent.width
                    text: "退出游戏"
                    accentColor: window.accentGreenMuted
                    selected: gameController.pauseMenuSelection === 1
                    onClicked: gameController.restartToMenu()
                }
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: window.gameOverVisible

        Rectangle {
            anchors.fill: parent
            color: "#78000000"
        }

        PanelSurface {
            anchors.centerIn: parent
            width: 344
            height: 234
            tint: window.panelRaised

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Text {
                    text: gameController.winnerText
                    color: window.textPrimary
                    font.pixelSize: 24
                    font.weight: Font.Medium
                }

                Row {
                    spacing: 10

                    StatPill {
                        label: gameController.mode === GameController.Pve ? gameController.displayPlayerName : "玩家1"
                        value: gameController.energy1
                        accentColor: window.accentGreen
                    }

                    StatPill {
                        label: gameController.mode === GameController.Pve ? "电脑" : "玩家2"
                        value: gameController.energy2
                        accentColor: window.accentGreenMuted
                    }
                }

                Text {
                    text: "使用下方按钮返回开始页。"
                    color: window.textSecondary
                    font.pixelSize: 12
                }

                AccentButton {
                    width: parent.width
                    text: "返回开始页"
                    accentColor: window.accentGreen
                    onClicked: gameController.restartToMenu()
                }
            }
        }
    }
}
