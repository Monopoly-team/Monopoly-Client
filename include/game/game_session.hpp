#pragma once

#include "game_state.hpp"

class GameSession
{
public:
    GameSession();

    GameState& state();
    const GameState& state() const;

private:
    GameState state_;
};