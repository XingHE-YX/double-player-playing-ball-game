#include "game/ai_controller.hpp"

#include <cmath>

#include "game/constants.hpp"

namespace {
float clampToArena(float y) {
    while (y < 0.0f || y > static_cast<float>(HEIGHT)) {
        if (y < 0.0f) {
            y = -y;
        }
        if (y > static_cast<float>(HEIGHT)) {
            y = 2.0f * HEIGHT - y;
        }
    }

    return y;
}
}

float AIController::computeTarget(const std::vector<Ball>& balls, const Paddle& aiPaddle,
    int aiEnergy, int playerEnergy, AIDifficulty difficulty) const {
    (void)aiEnergy;
    (void)playerEnergy;

    const Ball* threat = nullptr;
    float earliestTime = 1e9f;

    for (const auto& ball : balls) {
        if (ball.vx <= 0.0f) {
            continue;
        }

        const float distance = static_cast<float>(aiPaddle.x) - ball.x;
        if (distance < 0.0f) {
            continue;
        }

        const float timeToReach = distance / std::fabs(ball.vx + 0.001f);
        if (timeToReach < earliestTime) {
            earliestTime = timeToReach;
            threat = &ball;
        }
    }

    if (!threat) {
        return static_cast<float>(HEIGHT / 2);
    }

    const float exactPredictY = clampToArena(threat->y + threat->vy * earliestTime);
    float predictY = exactPredictY;

    switch (difficulty) {
    case AI_EASY:
        predictY = clampToArena(threat->y + threat->vy * earliestTime * 0.38f
            + (threat->vy >= 0.0f ? 84.0f : -84.0f));
        break;
    case AI_MEDIUM:
        predictY = clampToArena(threat->y + threat->vy * earliestTime * 0.72f
            + (threat->vy >= 0.0f ? 28.0f : -28.0f));
        break;
    case AI_HARD:
        predictY = exactPredictY;
        break;
    }

    return predictY;
}

float AIController::computeSpeed(int aiEnergy, int playerEnergy, AIDifficulty difficulty) const {
    const int gap = aiEnergy - playerEnergy;
    (void)gap;
    (void)difficulty;
    return static_cast<float>(PADDLE_SPEED);
}
