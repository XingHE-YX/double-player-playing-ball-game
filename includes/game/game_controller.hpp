#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <array>
#include <vector>

#include "game/ai_controller.hpp"
#include "game/ball.hpp"
#include "game/energy_bar.hpp"
#include "game/game_types.hpp"
#include "game/input_manager.hpp"
#include "game/paddle.hpp"

class GameController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int gameWidth READ gameWidth CONSTANT)
    Q_PROPERTY(int gameHeight READ gameHeight CONSTANT)
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(int mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(int menuSelection READ menuSelection NOTIFY menuSelectionChanged)
    Q_PROPERTY(int aiDifficulty READ aiDifficulty NOTIFY aiDifficultyChanged)
    Q_PROPERTY(QString playerName READ playerName WRITE setPlayerName NOTIFY playerNameChanged)
    Q_PROPERTY(QString displayPlayerName READ displayPlayerName NOTIFY playerNameChanged)
    Q_PROPERTY(QVariantList keyBindings READ keyBindings NOTIFY bindingsChanged)
    Q_PROPERTY(QVariantList leaderboard READ leaderboard NOTIFY leaderboardChanged)
    Q_PROPERTY(QString controlsHintText READ controlsHintText NOTIFY bindingsChanged)
    Q_PROPERTY(QString pauseKeyLabel READ pauseKeyLabel NOTIFY bindingsChanged)
    Q_PROPERTY(int pauseMenuSelection READ pauseMenuSelection NOTIFY pauseMenuSelectionChanged)
    Q_PROPERTY(int rebindingAction READ rebindingAction NOTIFY rebindingActionChanged)
    Q_PROPERTY(int paddle1X READ paddle1X CONSTANT)
    Q_PROPERTY(int paddle2X READ paddle2X CONSTANT)
    Q_PROPERTY(int paddleWidth READ paddleWidth CONSTANT)
    Q_PROPERTY(int paddleHeight READ paddleHeight CONSTANT)
    Q_PROPERTY(int paddle1Y READ paddle1Y NOTIFY sceneChanged)
    Q_PROPERTY(int paddle2Y READ paddle2Y NOTIFY sceneChanged)
    Q_PROPERTY(QVariantList balls READ balls NOTIFY sceneChanged)
    Q_PROPERTY(int energy1 READ energy1 NOTIFY sceneChanged)
    Q_PROPERTY(int energy2 READ energy2 NOTIFY sceneChanged)
    Q_PROPERTY(int bounceCount1 READ bounceCount1 NOTIFY sceneChanged)
    Q_PROPERTY(int bounceCount2 READ bounceCount2 NOTIFY sceneChanged)
    Q_PROPERTY(QString winnerText READ winnerText NOTIFY sceneChanged)
    Q_PROPERTY(QString modeLabel READ modeLabel NOTIFY sceneChanged)
    Q_PROPERTY(QString aiDifficultyLabel READ aiDifficultyLabel NOTIFY aiDifficultyChanged)

public:
    enum ViewState {
        Start = START,
        Running = RUNNING,
        Pause = PAUSE,
        GameOver = GAMEOVER
    };
    Q_ENUM(ViewState)

    enum ViewMode {
        Pvp = MODE_PVP,
        Pve = MODE_PVE
    };
    Q_ENUM(ViewMode)

    enum ViewAIDifficulty {
        AiEasy = AI_EASY,
        AiMedium = AI_MEDIUM,
        AiHard = AI_HARD
    };
    Q_ENUM(ViewAIDifficulty)

    enum ViewControlAction {
        ActionP1Up = 0,
        ActionP1Down,
        ActionP2Up,
        ActionP2Down,
        ActionPause
    };
    Q_ENUM(ViewControlAction)

    explicit GameController(QObject* parent = nullptr);

    int gameWidth() const;
    int gameHeight() const;
    int state() const;
    int mode() const;
    int menuSelection() const;
    int aiDifficulty() const;
    QString playerName() const;
    void setPlayerName(const QString& nextName);
    QString displayPlayerName() const;
    QVariantList keyBindings() const;
    QVariantList leaderboard() const;
    QString controlsHintText() const;
    QString pauseKeyLabel() const;
    int pauseMenuSelection() const;
    int rebindingAction() const;
    int paddle1X() const;
    int paddle2X() const;
    int paddleWidth() const;
    int paddleHeight() const;
    int paddle1Y() const;
    int paddle2Y() const;
    QVariantList balls() const;
    int energy1() const;
    int energy2() const;
    int bounceCount1() const;
    int bounceCount2() const;
    QString winnerText() const;
    QString modeLabel() const;
    QString aiDifficultyLabel() const;

    Q_INVOKABLE void pressKey(int key);
    Q_INVOKABLE void releaseKey(int key);
    Q_INVOKABLE void clearInput();
    Q_INVOKABLE void selectMenu(int index);
    Q_INVOKABLE void selectAiDifficulty(int difficulty);
    Q_INVOKABLE void beginRebinding(int action);
    Q_INVOKABLE void cancelRebinding();
    Q_INVOKABLE void resetLeaderboard();
    Q_INVOKABLE void setTextInputActive(bool active);
    Q_INVOKABLE void startSelectedMode();
    Q_INVOKABLE void startPvp();
    Q_INVOKABLE void startPve();
    Q_INVOKABLE void resumeGame();
    Q_INVOKABLE void quitGame();
    Q_INVOKABLE void restartToMenu();

signals:
    void stateChanged();
    void modeChanged();
    void menuSelectionChanged();
    void aiDifficultyChanged();
    void playerNameChanged();
    void bindingsChanged();
    void leaderboardChanged();
    void pauseMenuSelectionChanged();
    void rebindingActionChanged();
    void sceneChanged();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct LeaderboardEntry {
        QString name;
        int score;
        AIDifficulty difficulty;
        bool won;
        QString playedAt;
    };

    static constexpr int controlActionCount = 5;

    void tick();
    void emitSceneState();
    void setState(GameState nextState);
    void setMode(GameMode nextMode);
    void setMenuSelectionValue(int nextSelection);
    void setAiDifficultyValue(AIDifficulty nextDifficulty);
    void setPauseMenuSelectionValue(int nextSelection);
    void setRebindingAction(int nextAction);

    void loadSettings();
    void loadLeaderboard();
    void saveProfileSettings() const;
    void saveBindings() const;
    void saveLeaderboard() const;
    void restoreDefaultBindings();
    QString difficultyLabel(AIDifficulty difficulty) const;
    QString actionLabel(ViewControlAction action) const;
    QString keyLabel(int key) const;
    int bindingForAction(ViewControlAction action) const;
    void setBindingForAction(ViewControlAction action, int key);
    void recordLeaderboardEntry();

    void updateStart();
    void handlePaddleInput();
    void checkBallPaddleCollision(Ball& ball);
    int getEnergyBySize(int size) const;
    bool isSpawnPositionClear(const Ball& candidate, const Ball* ignoredBall = nullptr) const;
    bool placeBallAtSafeSpawn(Ball& ball, const Ball* ignoredBall = nullptr) const;
    Ball createSpawnBall() const;
    bool handleBallOut(int ballIndex);
    void checkBallMerge();
    void updateBalls();
    void updateRunning();
    void updatePause();
    void resetGame();
    void updateGameOver();

    GameState currentState;
    GameMode currentMode;
    int currentMenuSelection;
    AIDifficulty currentAiDifficulty;
    QString currentPlayerName;
    std::array<int, controlActionCount> controlBindings;
    std::vector<LeaderboardEntry> leaderboardEntries;
    int currentPauseMenuSelection;
    int currentRebindingAction;
    bool textInputActive;

    Paddle paddle1;
    Paddle paddle2;
    std::vector<Ball> liveBalls;
    EnergyBar energyBar1;
    EnergyBar energyBar2;
    InputManager input;
    AIController ai;

    int currentBounceCount1;
    int currentBounceCount2;
    int frameCount;
    int spawnTimer;
    int emptySpawnTimer;

    QTimer frameTimer;
};
