#include "game/game_controller.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMetaType>
#include <QSettings>

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "game/constants.hpp"

namespace {
constexpr float SPAWN_X_MIN_FACTOR = 0.34f;
constexpr float SPAWN_X_MAX_FACTOR = 0.66f;
constexpr float SPAWN_Y_MARGIN = 72.0f;
constexpr float SPAWN_CLEARANCE = 24.0f;
constexpr int SPAWN_ATTEMPTS = 32;
constexpr int LEADERBOARD_LIMIT = 10;

float randomFloat(float minValue, float maxValue) {
    const float ratio = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return minValue + (maxValue - minValue) * ratio;
}

QString bindingSettingsKey(int action) {
    switch (action) {
    case GameController::ActionP1Up:
        return QStringLiteral("bindings/p1Up");
    case GameController::ActionP1Down:
        return QStringLiteral("bindings/p1Down");
    case GameController::ActionP2Up:
        return QStringLiteral("bindings/p2Up");
    case GameController::ActionP2Down:
        return QStringLiteral("bindings/p2Down");
    case GameController::ActionPause:
        return QStringLiteral("bindings/pause");
    }

    return QString();
}

bool isModifierKey(int key) {
    return key == Qt::Key_Shift
        || key == Qt::Key_Control
        || key == Qt::Key_Meta
        || key == Qt::Key_Alt
        || key == Qt::Key_AltGr
        || key == Qt::Key_CapsLock
        || key == Qt::Key_NumLock
        || key == Qt::Key_ScrollLock;
}

}

GameController::GameController(QObject* parent)
    : QObject(parent),
      currentState(START),
      currentMode(MODE_PVP),
      currentMenuSelection(0),
      currentAiDifficulty(AI_MEDIUM),
      currentPlayerName(),
      controlBindings(),
      leaderboardEntries(),
      currentPauseMenuSelection(0),
      currentRebindingAction(-1),
      textInputActive(false),
      paddle1(P1_PADDLE_X),
      paddle2(P2_PADDLE_X),
      liveBalls(),
      energyBar1(1, 20, 30),
      energyBar2(2, 580, 30),
      currentBounceCount1(0),
      currentBounceCount2(0),
      frameCount(0),
      spawnTimer(BALL_SPAWN_INTERVAL),
      emptySpawnTimer(EMPTY_BALL_SPAWN_DELAY) {
    qRegisterMetaType<QVariantList>("QVariantList");

    restoreDefaultBindings();
    loadSettings();
    loadLeaderboard();
    liveBalls.push_back(createSpawnBall());

    frameTimer.setTimerType(Qt::PreciseTimer);
    frameTimer.setInterval(1000 / FPS);
    connect(&frameTimer, &QTimer::timeout, this, [this]() { tick(); });
    frameTimer.start();
}

int GameController::gameWidth() const {
    return WIDTH;
}

int GameController::gameHeight() const {
    return HEIGHT;
}

int GameController::state() const {
    return currentState;
}

int GameController::mode() const {
    return currentMode;
}

int GameController::menuSelection() const {
    return currentMenuSelection;
}

int GameController::aiDifficulty() const {
    return currentAiDifficulty;
}

QString GameController::playerName() const {
    return currentPlayerName;
}

void GameController::setPlayerName(const QString& nextName) {
    QString sanitized = nextName;
    sanitized.replace(QLatin1Char('\n'), QLatin1Char(' '));
    sanitized.replace(QLatin1Char('\r'), QLatin1Char(' '));
    sanitized = sanitized.left(8);

    if (currentPlayerName == sanitized) {
        return;
    }

    currentPlayerName = sanitized;
    saveProfileSettings();
    emit playerNameChanged();
    emit sceneChanged();
}

QString GameController::displayPlayerName() const {
    const QString trimmed = currentPlayerName.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("玩家") : trimmed;
}

QVariantList GameController::keyBindings() const {
    QVariantList bindings;
    bindings.reserve(controlActionCount);

    for (int action = ActionP1Up; action <= ActionPause; ++action) {
        QVariantMap item;
        item.insert(QStringLiteral("action"), action);
        item.insert(QStringLiteral("label"), actionLabel(static_cast<ViewControlAction>(action)));
        item.insert(QStringLiteral("keyLabel"), keyLabel(bindingForAction(static_cast<ViewControlAction>(action))));
        bindings.push_back(item);
    }

    return bindings;
}

QVariantList GameController::leaderboard() const {
    QVariantList rows;
    rows.reserve(static_cast<qsizetype>(leaderboardEntries.size()));

    for (const auto& entry : leaderboardEntries) {
        QVariantMap item;
        item.insert(QStringLiteral("name"), entry.name);
        item.insert(QStringLiteral("score"), entry.score);
        item.insert(QStringLiteral("difficulty"), difficultyLabel(entry.difficulty));
        item.insert(QStringLiteral("result"), entry.won ? QStringLiteral("胜利") : QStringLiteral("失利"));

        const QDateTime playedAt = QDateTime::fromString(entry.playedAt, Qt::ISODate);
        item.insert(QStringLiteral("playedAt"),
            playedAt.isValid() ? playedAt.toString(QStringLiteral("MM-dd HH:mm")) : entry.playedAt);
        rows.push_back(item);
    }

    return rows;
}

QString GameController::controlsHintText() const {
    return QStringLiteral("P1 %1/%2    P2 %3/%4    暂停 %5")
        .arg(keyLabel(bindingForAction(ActionP1Up)))
        .arg(keyLabel(bindingForAction(ActionP1Down)))
        .arg(keyLabel(bindingForAction(ActionP2Up)))
        .arg(keyLabel(bindingForAction(ActionP2Down)))
        .arg(pauseKeyLabel());
}

QString GameController::pauseKeyLabel() const {
    return keyLabel(bindingForAction(ActionPause));
}

int GameController::pauseMenuSelection() const {
    return currentPauseMenuSelection;
}

int GameController::rebindingAction() const {
    return currentRebindingAction;
}

int GameController::paddle1X() const {
    return paddle1.x;
}

int GameController::paddle2X() const {
    return paddle2.x;
}

int GameController::paddleWidth() const {
    return paddle1.width;
}

int GameController::paddleHeight() const {
    return paddle1.height;
}

int GameController::paddle1Y() const {
    return paddle1.y;
}

int GameController::paddle2Y() const {
    return paddle2.y;
}

QVariantList GameController::balls() const {
    QVariantList ballList;
    ballList.reserve(static_cast<qsizetype>(liveBalls.size()));

    for (const auto& ball : liveBalls) {
        QVariantMap item;
        item.insert(QStringLiteral("x"), ball.x);
        item.insert(QStringLiteral("y"), ball.y);
        item.insert(QStringLiteral("size"), ball.size);
        ballList.push_back(item);
    }

    return ballList;
}

int GameController::energy1() const {
    return energyBar1.energy;
}

int GameController::energy2() const {
    return energyBar2.energy;
}

int GameController::bounceCount1() const {
    return currentBounceCount1;
}

int GameController::bounceCount2() const {
    return currentBounceCount2;
}

QString GameController::winnerText() const {
    if (currentMode == MODE_PVE) {
        return energyBar1.isFull() ? QStringLiteral("电脑获胜") : displayPlayerName() + QStringLiteral(" 获胜");
    }

    return energyBar1.isFull() ? QStringLiteral("玩家 2 获胜") : QStringLiteral("玩家 1 获胜");
}

QString GameController::modeLabel() const {
    return currentMode == MODE_PVP ? QStringLiteral("双人") : QStringLiteral("人机");
}

QString GameController::aiDifficultyLabel() const {
    return difficultyLabel(currentAiDifficulty);
}

void GameController::pressKey(int key) {
    input.pressKey(key);
}

void GameController::releaseKey(int key) {
    input.releaseKey(key);
}

void GameController::clearInput() {
    input.clear();
}

void GameController::selectMenu(int index) {
    setMenuSelectionValue(index == 0 ? 0 : 1);
}

void GameController::selectAiDifficulty(int difficulty) {
    if (difficulty <= AI_EASY) {
        setAiDifficultyValue(AI_EASY);
        return;
    }
    if (difficulty >= AI_HARD) {
        setAiDifficultyValue(AI_HARD);
        return;
    }

    setAiDifficultyValue(AI_MEDIUM);
}

void GameController::beginRebinding(int action) {
    if (action < ActionP1Up || action > ActionPause) {
        return;
    }

    input.clear();
    textInputActive = false;
    setRebindingAction(currentRebindingAction == action ? -1 : action);
}

void GameController::cancelRebinding() {
    setRebindingAction(-1);
}

void GameController::resetLeaderboard() {
    if (leaderboardEntries.empty()) {
        return;
    }

    leaderboardEntries.clear();
    saveLeaderboard();
    emit leaderboardChanged();
}

void GameController::setTextInputActive(bool active) {
    if (textInputActive == active) {
        return;
    }

    textInputActive = active;
    if (textInputActive) {
        input.clear();
    }
}

void GameController::startSelectedMode() {
    if (currentMenuSelection == 0) {
        startPvp();
    }
    else {
        startPve();
    }
}

void GameController::startPvp() {
    cancelRebinding();
    setMenuSelectionValue(0);
    setMode(MODE_PVP);
    liveBalls.clear();
    liveBalls.push_back(createSpawnBall());
    spawnTimer = BALL_SPAWN_INTERVAL;
    emptySpawnTimer = EMPTY_BALL_SPAWN_DELAY;
    input.clear();
    setState(RUNNING);
    emitSceneState();
}

void GameController::startPve() {
    cancelRebinding();
    setMenuSelectionValue(1);
    setMode(MODE_PVE);
    liveBalls.clear();
    liveBalls.push_back(createSpawnBall());
    spawnTimer = BALL_SPAWN_INTERVAL;
    emptySpawnTimer = EMPTY_BALL_SPAWN_DELAY;
    input.clear();
    setState(RUNNING);
    emitSceneState();
}

void GameController::resumeGame() {
    if (currentState == PAUSE) {
        setState(RUNNING);
        emitSceneState();
    }
}

void GameController::quitGame() {
    QCoreApplication::quit();
}

void GameController::restartToMenu() {
    resetGame();
    emitSceneState();
}

bool GameController::eventFilter(QObject* watched, QEvent* event) {
    (void)watched;

    switch (event->type()) {
    case QEvent::KeyPress: {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (currentRebindingAction != -1) {
            if (!keyEvent->isAutoRepeat() && !isModifierKey(keyEvent->key())) {
                setBindingForAction(static_cast<ViewControlAction>(currentRebindingAction), keyEvent->key());
                setRebindingAction(-1);
            }
            return true;
        }

        if (textInputActive) {
            return false;
        }

        if (!keyEvent->isAutoRepeat()) {
            input.pressKey(keyEvent->key());
        }
        break;
    }
    case QEvent::KeyRelease: {
        if (currentRebindingAction != -1) {
            return true;
        }
        if (textInputActive) {
            return false;
        }

        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (!keyEvent->isAutoRepeat()) {
            input.releaseKey(keyEvent->key());
        }
        break;
    }
    case QEvent::ApplicationDeactivate:
    case QEvent::WindowDeactivate:
        input.clear();
        break;
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

void GameController::tick() {
    switch (currentState) {
    case START:
        updateStart();
        break;
    case RUNNING:
        updateRunning();
        break;
    case PAUSE:
        updatePause();
        break;
    case GAMEOVER:
        updateGameOver();
        break;
    }

    frameCount++;
    input.endFrame();
    emit sceneChanged();
}

void GameController::emitSceneState() {
    emit stateChanged();
    emit modeChanged();
    emit menuSelectionChanged();
    emit sceneChanged();
}

void GameController::setState(GameState nextState) {
    if (currentState == nextState) {
        return;
    }

    currentState = nextState;
    emit stateChanged();
}

void GameController::setMode(GameMode nextMode) {
    if (currentMode == nextMode) {
        return;
    }

    currentMode = nextMode;
    emit modeChanged();
}

void GameController::setMenuSelectionValue(int nextSelection) {
    if (currentMenuSelection == nextSelection) {
        return;
    }

    currentMenuSelection = nextSelection;
    emit menuSelectionChanged();
}

void GameController::setAiDifficultyValue(AIDifficulty nextDifficulty) {
    if (currentAiDifficulty == nextDifficulty) {
        return;
    }

    currentAiDifficulty = nextDifficulty;
    saveProfileSettings();
    emit aiDifficultyChanged();
}

void GameController::setPauseMenuSelectionValue(int nextSelection) {
    const int normalizedSelection = nextSelection == 0 ? 0 : 1;
    if (currentPauseMenuSelection == normalizedSelection) {
        return;
    }

    currentPauseMenuSelection = normalizedSelection;
    emit pauseMenuSelectionChanged();
}

void GameController::setRebindingAction(int nextAction) {
    if (currentRebindingAction == nextAction) {
        return;
    }

    currentRebindingAction = nextAction;
    emit rebindingActionChanged();
}

void GameController::loadSettings() {
    QSettings settings;

    currentPlayerName = settings.value(QStringLiteral("profile/name"), QString()).toString();

    const int savedDifficulty = settings.value(QStringLiteral("profile/aiDifficulty"),
        static_cast<int>(AI_MEDIUM)).toInt();
    if (savedDifficulty <= AI_EASY) {
        currentAiDifficulty = AI_EASY;
    }
    else if (savedDifficulty >= AI_HARD) {
        currentAiDifficulty = AI_HARD;
    }
    else {
        currentAiDifficulty = AI_MEDIUM;
    }

    for (int action = ActionP1Up; action <= ActionPause; ++action) {
        const QString key = bindingSettingsKey(action);
        if (!key.isEmpty()) {
            controlBindings[action] = settings.value(key, controlBindings[action]).toInt();
        }
    }
}

void GameController::loadLeaderboard() {
    QSettings settings;
    const int size = settings.beginReadArray(QStringLiteral("leaderboard"));
    leaderboardEntries.clear();
    leaderboardEntries.reserve(size);

    for (int index = 0; index < size; ++index) {
        settings.setArrayIndex(index);

        LeaderboardEntry entry;
        entry.name = settings.value(QStringLiteral("name"), QStringLiteral("玩家")).toString();
        entry.score = settings.value(QStringLiteral("score"), 0).toInt();

        const int difficulty = settings.value(QStringLiteral("difficulty"), static_cast<int>(AI_MEDIUM)).toInt();
        if (difficulty <= AI_EASY) {
            entry.difficulty = AI_EASY;
        }
        else if (difficulty >= AI_HARD) {
            entry.difficulty = AI_HARD;
        }
        else {
            entry.difficulty = AI_MEDIUM;
        }

        entry.won = settings.value(QStringLiteral("won"), false).toBool();
        entry.playedAt = settings.value(QStringLiteral("playedAt"), QString()).toString();
        leaderboardEntries.push_back(entry);
    }

    settings.endArray();

    std::sort(leaderboardEntries.begin(), leaderboardEntries.end(),
        [](const auto& lhs, const auto& rhs) {
            if (lhs.score != rhs.score) {
                return lhs.score > rhs.score;
            }
            if (lhs.difficulty != rhs.difficulty) {
                return lhs.difficulty > rhs.difficulty;
            }
            if (lhs.won != rhs.won) {
                return lhs.won && !rhs.won;
            }
            return lhs.playedAt > rhs.playedAt;
        });
    if (static_cast<int>(leaderboardEntries.size()) > LEADERBOARD_LIMIT) {
        leaderboardEntries.resize(LEADERBOARD_LIMIT);
    }
}

void GameController::saveProfileSettings() const {
    QSettings settings;
    settings.setValue(QStringLiteral("profile/name"), currentPlayerName);
    settings.setValue(QStringLiteral("profile/aiDifficulty"), static_cast<int>(currentAiDifficulty));
}

void GameController::saveBindings() const {
    QSettings settings;

    for (int action = ActionP1Up; action <= ActionPause; ++action) {
        const QString key = bindingSettingsKey(action);
        if (!key.isEmpty()) {
            settings.setValue(key, controlBindings[action]);
        }
    }
}

void GameController::saveLeaderboard() const {
    QSettings settings;
    settings.remove(QStringLiteral("leaderboard"));
    settings.beginWriteArray(QStringLiteral("leaderboard"), static_cast<int>(leaderboardEntries.size()));

    for (int index = 0; index < static_cast<int>(leaderboardEntries.size()); ++index) {
        settings.setArrayIndex(index);
        const auto& entry = leaderboardEntries[index];
        settings.setValue(QStringLiteral("name"), entry.name);
        settings.setValue(QStringLiteral("score"), entry.score);
        settings.setValue(QStringLiteral("difficulty"), static_cast<int>(entry.difficulty));
        settings.setValue(QStringLiteral("won"), entry.won);
        settings.setValue(QStringLiteral("playedAt"), entry.playedAt);
    }

    settings.endArray();
}

void GameController::restoreDefaultBindings() {
    controlBindings[ActionP1Up] = Qt::Key_W;
    controlBindings[ActionP1Down] = Qt::Key_S;
    controlBindings[ActionP2Up] = Qt::Key_I;
    controlBindings[ActionP2Down] = Qt::Key_K;
    controlBindings[ActionPause] = Qt::Key_Escape;
}

QString GameController::difficultyLabel(AIDifficulty difficulty) const {
    switch (difficulty) {
    case AI_EASY:
        return QStringLiteral("简单");
    case AI_MEDIUM:
        return QStringLiteral("普通");
    case AI_HARD:
        return QStringLiteral("困难");
    }

    return QStringLiteral("普通");
}

QString GameController::actionLabel(ViewControlAction action) const {
    switch (action) {
    case ActionP1Up:
        return QStringLiteral("玩家1 上移");
    case ActionP1Down:
        return QStringLiteral("玩家1 下移");
    case ActionP2Up:
        return QStringLiteral("玩家2 上移");
    case ActionP2Down:
        return QStringLiteral("玩家2 下移");
    case ActionPause:
        return QStringLiteral("暂停 / 继续");
    }

    return QString();
}

QString GameController::keyLabel(int key) const {
    const QString sequence = QKeySequence(key).toString(QKeySequence::NativeText);
    if (!sequence.isEmpty()) {
        return sequence.toUpper();
    }

    return QStringLiteral("未设置");
}

int GameController::bindingForAction(ViewControlAction action) const {
    return controlBindings[action];
}

void GameController::setBindingForAction(ViewControlAction action, int key) {
    if (controlBindings[action] == key) {
        return;
    }

    controlBindings[action] = key;
    saveBindings();
    emit bindingsChanged();
    emit sceneChanged();
}

void GameController::recordLeaderboardEntry() {
    if (currentMode != MODE_PVE) {
        return;
    }

    LeaderboardEntry entry;
    entry.name = displayPlayerName();
    entry.score = currentBounceCount1;
    entry.difficulty = currentAiDifficulty;
    entry.won = energyBar2.isFull();
    entry.playedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    leaderboardEntries.push_back(entry);

    std::sort(leaderboardEntries.begin(), leaderboardEntries.end(),
        [](const auto& lhs, const auto& rhs) {
            if (lhs.score != rhs.score) {
                return lhs.score > rhs.score;
            }
            if (lhs.difficulty != rhs.difficulty) {
                return lhs.difficulty > rhs.difficulty;
            }
            if (lhs.won != rhs.won) {
                return lhs.won && !rhs.won;
            }
            return lhs.playedAt > rhs.playedAt;
        });
    if (static_cast<int>(leaderboardEntries.size()) > LEADERBOARD_LIMIT) {
        leaderboardEntries.resize(LEADERBOARD_LIMIT);
    }

    saveLeaderboard();
    emit leaderboardChanged();
}

void GameController::updateStart() {
    if (currentRebindingAction != -1 || textInputActive) {
        return;
    }

    if (input.isKeyPressed(Qt::Key_Up)) {
        setMenuSelectionValue(currentMenuSelection == 0 ? 1 : 0);
    }
    if (input.isKeyPressed(Qt::Key_Down)) {
        setMenuSelectionValue(currentMenuSelection == 0 ? 1 : 0);
    }
    if (currentMenuSelection == 1 && input.isKeyPressed(Qt::Key_Left)) {
        const int nextDifficulty = std::max(static_cast<int>(AI_EASY), static_cast<int>(currentAiDifficulty) - 1);
        setAiDifficultyValue(static_cast<AIDifficulty>(nextDifficulty));
    }
    if (currentMenuSelection == 1 && input.isKeyPressed(Qt::Key_Right)) {
        const int nextDifficulty = std::min(static_cast<int>(AI_HARD), static_cast<int>(currentAiDifficulty) + 1);
        setAiDifficultyValue(static_cast<AIDifficulty>(nextDifficulty));
    }
    if (input.isKeyPressed(Qt::Key_Return) || input.isKeyPressed(Qt::Key_Enter)) {
        startSelectedMode();
    }
}

void GameController::handlePaddleInput() {
    if (input.isKeyDown(bindingForAction(ActionP1Up))) {
        paddle1.moveUp();
    }
    if (input.isKeyDown(bindingForAction(ActionP1Down))) {
        paddle1.moveDown();
    }

    if (currentMode == MODE_PVP) {
        if (input.isKeyDown(bindingForAction(ActionP2Up))) {
            paddle2.moveUp();
        }
        if (input.isKeyDown(bindingForAction(ActionP2Down))) {
            paddle2.moveDown();
        }
    }
    else {
        const float targetY = ai.computeTarget(liveBalls, paddle2, energyBar2.energy, energyBar1.energy, currentAiDifficulty);
        const float speed = ai.computeSpeed(energyBar2.energy, energyBar1.energy, currentAiDifficulty);
        paddle2.moveToward(targetY, speed);
    }
}

void GameController::checkBallPaddleCollision(Ball& ball) {
    const auto r1 = paddle1.rect();
    if (ball.distToRect(r1) <= static_cast<float>(ball.size)) {
        ball.vx = std::fabs(ball.vx);
        const float off = (ball.y - (paddle1.y + PADDLE_H / 2.0f)) / (PADDLE_H / 2.0f);
        ball.vy = off * 6.0f;
        if (std::fabs(ball.vy) < 1.5f) {
            ball.vy = ball.vy < 0.0f ? -1.5f : 1.5f;
        }
        ball.x = static_cast<float>(paddle1.x + paddle1.width + ball.size + 1);
        currentBounceCount1++;
    }

    const auto r2 = paddle2.rect();
    if (ball.distToRect(r2) <= static_cast<float>(ball.size)) {
        ball.vx = -std::fabs(ball.vx);
        const float off = (ball.y - (paddle2.y + PADDLE_H / 2.0f)) / (PADDLE_H / 2.0f);
        ball.vy = off * 6.0f;
        if (std::fabs(ball.vy) < 1.5f) {
            ball.vy = ball.vy < 0.0f ? -1.5f : 1.5f;
        }
        ball.x = static_cast<float>(paddle2.x - ball.size - 1);
        currentBounceCount2++;
    }
}

int GameController::getEnergyBySize(int size) const {
    if (size < 10) {
        return 1;
    }
    if (size < 18) {
        return 2;
    }
    return 4;
}

bool GameController::isSpawnPositionClear(const Ball& candidate, const Ball* ignoredBall) const {
    if (candidate.distToRect(paddle1.rect()) <= static_cast<float>(candidate.size) + SPAWN_CLEARANCE) {
        return false;
    }

    if (candidate.distToRect(paddle2.rect()) <= static_cast<float>(candidate.size) + SPAWN_CLEARANCE) {
        return false;
    }

    for (const auto& liveBall : liveBalls) {
        if (&liveBall == ignoredBall) {
            continue;
        }

        const float dx = candidate.x - liveBall.x;
        const float dy = candidate.y - liveBall.y;
        const float minDistance = static_cast<float>(candidate.size + liveBall.size) + SPAWN_CLEARANCE;

        if (std::sqrt(dx * dx + dy * dy) <= minDistance) {
            return false;
        }
    }

    return true;
}

bool GameController::placeBallAtSafeSpawn(Ball& ball, const Ball* ignoredBall) const {
    const float minX = static_cast<float>(WIDTH) * SPAWN_X_MIN_FACTOR;
    const float maxX = static_cast<float>(WIDTH) * SPAWN_X_MAX_FACTOR;
    const float minY = SPAWN_Y_MARGIN;
    const float maxY = static_cast<float>(HEIGHT) - SPAWN_Y_MARGIN;

    for (int attempt = 0; attempt < SPAWN_ATTEMPTS; ++attempt) {
        ball.x = randomFloat(minX, maxX);
        ball.y = randomFloat(minY, maxY);
        ball.size = 6;
        ball.randomVelocity();

        if (isSpawnPositionClear(ball, ignoredBall)) {
            return true;
        }
    }

    constexpr int fallbackColumns = 5;
    constexpr int fallbackRows = 4;

    for (int row = 0; row < fallbackRows; ++row) {
        const float rowFactor = fallbackRows == 1 ? 0.0f : static_cast<float>(row) / static_cast<float>(fallbackRows - 1);
        for (int column = 0; column < fallbackColumns; ++column) {
            const float columnFactor = fallbackColumns == 1 ? 0.0f : static_cast<float>(column) / static_cast<float>(fallbackColumns - 1);
            ball.x = minX + (maxX - minX) * columnFactor;
            ball.y = minY + (maxY - minY) * rowFactor;
            ball.size = 6;
            ball.randomVelocity();

            if (isSpawnPositionClear(ball, ignoredBall)) {
                return true;
            }
        }
    }

    return false;
}

Ball GameController::createSpawnBall() const {
    Ball spawnedBall;

    if (placeBallAtSafeSpawn(spawnedBall)) {
        return spawnedBall;
    }

    spawnedBall.reset();
    return spawnedBall;
}

bool GameController::handleBallOut(int ballIndex) {
    if (ballIndex < 0 || ballIndex >= static_cast<int>(liveBalls.size())) {
        return false;
    }

    const Ball& ball = liveBalls[ballIndex];
    if (!ball.isOutLeft() && !ball.isOutRight()) {
        return false;
    }

    if (ball.isOutLeft()) {
        energyBar1.addEnergy(getEnergyBySize(ball.size));
    }
    else {
        energyBar2.addEnergy(getEnergyBySize(ball.size));
    }

    liveBalls.erase(liveBalls.begin() + ballIndex);
    return true;
}

void GameController::checkBallMerge() {
    for (int i = 0; i < static_cast<int>(liveBalls.size()); i++) {
        for (int j = i + 1; j < static_cast<int>(liveBalls.size()); j++) {
            const float dx = liveBalls[i].x - liveBalls[j].x;
            const float dy = liveBalls[i].y - liveBalls[j].y;

            if (std::sqrt(dx * dx + dy * dy) <= liveBalls[i].size + liveBalls[j].size) {
                int newSize = liveBalls[i].size + liveBalls[j].size;
                if (newSize > BALL_MAX_SIZE) {
                    newSize = BALL_MAX_SIZE;
                }

                float avgVx = (liveBalls[i].vx + liveBalls[j].vx) / 2.0f;
                float avgVy = (liveBalls[i].vy + liveBalls[j].vy) / 2.0f;
                const float speed = std::sqrt(avgVx * avgVx + avgVy * avgVy);

                if (speed < static_cast<float>(BALL_SPEED_MIN)) {
                    const float scale = static_cast<float>(BALL_SPEED_MIN) / (speed + 0.001f);
                    avgVx *= scale;
                    avgVy *= scale;
                }

                liveBalls[i].vx = avgVx;
                liveBalls[i].vy = avgVy;
                liveBalls[i].size = newSize;
                liveBalls.erase(liveBalls.begin() + j);
                j--;
            }
        }
    }
}

void GameController::updateBalls() {
    bool ballLostThisFrame = false;

    for (int index = 0; index < static_cast<int>(liveBalls.size());) {
        auto& ball = liveBalls[index];
        ball.move();
        ball.bounceWalls();
        checkBallPaddleCollision(ball);

        if (handleBallOut(index)) {
            ballLostThisFrame = true;
            continue;
        }

        index++;
    }

    checkBallMerge();

    if (liveBalls.empty()) {
        if (ballLostThisFrame) {
            emptySpawnTimer = EMPTY_BALL_SPAWN_DELAY;
            return;
        }

        emptySpawnTimer--;
        if (emptySpawnTimer <= 0) {
            liveBalls.push_back(createSpawnBall());
            spawnTimer = BALL_SPAWN_INTERVAL;
            emptySpawnTimer = EMPTY_BALL_SPAWN_DELAY;
        }
        return;
    }

    emptySpawnTimer = EMPTY_BALL_SPAWN_DELAY;

    if (ballLostThisFrame) {
        // Losing a ball should buy time before the next spawn, not instantly replace it.
        spawnTimer = BALL_SPAWN_INTERVAL;
        return;
    }

    if (static_cast<int>(liveBalls.size()) < MAX_BALL_COUNT) {
        spawnTimer--;
        if (spawnTimer <= 0) {
            liveBalls.push_back(createSpawnBall());
            spawnTimer = BALL_SPAWN_INTERVAL;
        }
    }
    else {
        spawnTimer = BALL_SPAWN_INTERVAL;
    }
}

void GameController::updateRunning() {
    if (input.isKeyPressed(bindingForAction(ActionPause))) {
        setPauseMenuSelectionValue(0);
        setState(PAUSE);
        emitSceneState();
        return;
    }

    handlePaddleInput();
    updateBalls();

    if (energyBar1.isFull() || energyBar2.isFull()) {
        recordLeaderboardEntry();
        setState(GAMEOVER);
        emitSceneState();
    }
}

void GameController::updatePause() {
    if (input.isKeyPressed(bindingForAction(ActionPause))) {
        setState(RUNNING);
        emitSceneState();
        return;
    }
    if (input.isKeyPressed(Qt::Key_Up) || input.isKeyPressed(Qt::Key_Down)
        || input.isKeyPressed(Qt::Key_Left) || input.isKeyPressed(Qt::Key_Right)) {
        setPauseMenuSelectionValue(currentPauseMenuSelection == 0 ? 1 : 0);
        return;
    }
    if (input.isKeyPressed(Qt::Key_Return) || input.isKeyPressed(Qt::Key_Enter)) {
        if (currentPauseMenuSelection == 0) {
            resumeGame();
        }
        else {
            restartToMenu();
        }
        return;
    }
    if (input.isKeyPressed(Qt::Key_Q)) {
        quitGame();
    }
}

void GameController::resetGame() {
    cancelRebinding();
    liveBalls.clear();
    liveBalls.push_back(createSpawnBall());
    energyBar1.reset();
    energyBar2.reset();
    currentBounceCount1 = 0;
    currentBounceCount2 = 0;
    frameCount = 0;
    spawnTimer = BALL_SPAWN_INTERVAL;
    emptySpawnTimer = EMPTY_BALL_SPAWN_DELAY;
    input.clear();
    setMenuSelectionValue(0);
    setState(START);
}

void GameController::updateGameOver() {
}
