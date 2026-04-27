#pragma once

#include <vector>

#include "game/ball.hpp"
#include "game/game_types.hpp"
#include "game/paddle.hpp"

class AIController {
public:
    float computeTarget(const std::vector<Ball>& balls, const Paddle& aiPaddle,
        int aiEnergy, int playerEnergy, AIDifficulty difficulty) const;
    float computeSpeed(int aiEnergy, int playerEnergy, AIDifficulty difficulty) const;
};
