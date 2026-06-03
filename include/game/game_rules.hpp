#pragma once

#include "game/game_state.hpp"

class GameRules final
{
public:
    GameRules() = delete;

    static bool isBusinessCell(const Cell& cell);

    static bool canBuyCell(const Player& player, const Cell& cell);
    static bool canBuildHouse(const GameState& state, const Player& player, const Cell& cell);

    static bool canBuyBusiness(const Player& player, const Cell& cell);
    static bool canBuildBusiness(const GameState& state, const Player& player, const Cell& cell);

    static int calculateRent(const GameState& state, const Cell& cell);
    static bool checkBankruptcy(const Player& player);

    static bool ownsFullGroup(const GameState& state, int ownerId, BusinessGroup group);
    static int ownedExtraBusinessCount(const GameState& state, int ownerId);

    static int nearestBusinessCellIdFrom(const GameState& state, int position);
    static int propertyMaintenanceCost(const GameState& state, const Player& player, int costPerProperty);
};