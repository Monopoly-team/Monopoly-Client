#include "game/game_rules.hpp"

#include <algorithm>
#include <limits>

namespace {

int clampValue(int value, int minValue, int maxValue)
{
    if (value < minValue) {
        return minValue;
    }

    if (value > maxValue) {
        return maxValue;
    }

    return value;
}

int normalizedBoardPosition(int position, int boardSize)
{
    if (boardSize <= 0) {
        return -1;
    }

    int normalizedPosition = position % boardSize;

    if (normalizedPosition < 0) {
        normalizedPosition += boardSize;
    }

    return normalizedPosition;
}
int scaledValue(int value, int numerator, int denominator)
{
    if (denominator == 0) {
        return value;
    }

    return (value * numerator + denominator / 2) / denominator;
}

int groupBusinessCount(const GameState& state, BusinessGroup group)
{
    int count = 0;

    for (const Cell& cell : state.board()) {
        if (cell.type == CellType::Business && cell.group == group) {
            ++count;
        }
    }

    return count;
}

int ownedGroupBusinessCount(const GameState& state, int ownerId, BusinessGroup group)
{
    int count = 0;

    for (const Cell& cell : state.board()) {
        if (cell.type == CellType::Business &&
            cell.group == group &&
            cell.ownerId == ownerId)
        {
            ++count;
        }
    }

    return count;
}
} // namespace

bool GameRules::isBusinessCell(const Cell& cell)
{
    return cell.type == CellType::Business || cell.type == CellType::ExtraBusiness;
}

bool GameRules::canBuyCell(const Player& player, const Cell& cell)
{
    return isBusinessCell(cell)
           && cell.ownerId == NO_OWNER_ID
           && cell.price > 0
           && player.balance >= cell.price
           && !player.isBankrupt
           && player.active;
}

bool GameRules::canBuildHouse(const GameState& state, const Player& player, const Cell& cell)
{
    return cell.type == CellType::Business
           && cell.ownerId == player.id
           && cell.buildingCost > 0
           && cell.maxBuildingLevel > 0
           && cell.buildingLevel < cell.maxBuildingLevel
           && player.balance >= calculateNextBuildingCost(cell)
           && ownsFullGroup(state, player.id, cell.group)
           && canBuildEvenly(state, cell)
           && !player.isBankrupt
           && player.active;
}

bool GameRules::canBuyBusiness(const Player& player, const Cell& cell)
{
    return canBuyCell(player, cell);
}

bool GameRules::canBuildBusiness(const GameState& state, const Player& player, const Cell& cell)
{
    return canBuildHouse(state, player, cell);
}

bool GameRules::canBuildEvenly(const GameState& state, const Cell& targetCell)
{
    if (targetCell.type != CellType::Business)
        return false;

    if (targetCell.group == BusinessGroup::None)
        return false;

    if (targetCell.ownerId == NO_OWNER_ID)
        return false;

    int minBuildingLevel = std::numeric_limits<int>::max();
    int maxBuildingLevel = std::numeric_limits<int>::min();

    bool hasGroupCells = false;

    for (const Cell& cell : state.board())
    {
        if (cell.type != CellType::Business || cell.group != targetCell.group)
            continue;

        if (cell.ownerId != targetCell.ownerId)
            return false;

        hasGroupCells = true;

        const int nextLevel =
            cell.id == targetCell.id
                ? cell.buildingLevel + 1
                : cell.buildingLevel;

        minBuildingLevel = std::min(minBuildingLevel, nextLevel);
        maxBuildingLevel = std::max(maxBuildingLevel, nextLevel);
    }

    if (!hasGroupCells)
        return false;

    return maxBuildingLevel - minBuildingLevel <= 1;
}

int GameRules::calculateRent(const GameState& state, const Cell& cell)
{
    if (cell.ownerId == NO_OWNER_ID || cell.rentLevels.isEmpty()) {
        return 0;
    }

    if (cell.type == CellType::ExtraBusiness) {
        int businessCount = ownedExtraBusinessCount(state, cell.ownerId);

        if (businessCount < 1) {
            businessCount = 1;
        }

        const int rentIndex = clampValue(
            businessCount - 1,
            0,
            cell.rentLevels.size() - 1
            );

        return cell.rentLevels[rentIndex];
    }

    if (cell.type != CellType::Business) {
        return 0;
    }

    const int buildingLevel = clampValue(
        cell.buildingLevel,
        0,
        cell.rentLevels.size() - 1
        );

    int rent = cell.rentLevels[buildingLevel];

    if (buildingLevel == 0) {
        const int totalInGroup = groupBusinessCount(state, cell.group);
        const int ownedInGroup = ownedGroupBusinessCount(state, cell.ownerId, cell.group);

        if (totalInGroup >= 3 && ownedInGroup >= totalInGroup) {
            rent *= 2;
        } else if (totalInGroup >= 3 && ownedInGroup >= 2) {
            rent = scaledValue(rent, 3, 2);
        }
    }

    return rent;
}

int GameRules::calculateNextBuildingCost(const Cell& cell)
{
    if (cell.type != CellType::Business) {
        return 0;
    }

    if (cell.buildingCost <= 0 || cell.maxBuildingLevel <= 0) {
        return 0;
    }

    if (cell.buildingLevel >= cell.maxBuildingLevel) {
        return 0;
    }

    const int nextLevel = cell.buildingLevel + 1;

    switch (nextLevel) {
    case 1:
        return cell.buildingCost;                           // x1
    case 2:
        return scaledValue(cell.buildingCost, 133, 100);    // x1.33
    case 3:
        return scaledValue(cell.buildingCost, 3, 2);        // x1.5
    case 4:
        return scaledValue(cell.buildingCost, 7, 4);        // x1.75
    default:
        return cell.buildingCost * 2;                       // hotel x2
    }
}

bool GameRules::checkBankruptcy(const Player& player)
{
    return player.isBankrupt || player.balance < 0;
}

bool GameRules::ownsFullGroup(const GameState& state, int ownerId, BusinessGroup group)
{
    if (ownerId == NO_OWNER_ID || group == BusinessGroup::None) {
        return false;
    }

    bool hasGroupCells = false;

    for (const Cell& cell : state.board()) {
        if (cell.type != CellType::Business || cell.group != group) {
            continue;
        }

        hasGroupCells = true;

        if (cell.ownerId != ownerId) {
            return false;
        }
    }

    return hasGroupCells;
}

int GameRules::ownedExtraBusinessCount(const GameState& state, int ownerId)
{
    if (ownerId == NO_OWNER_ID) {
        return 0;
    }

    int count = 0;

    for (const Cell& cell : state.board()) {
        if (cell.type == CellType::ExtraBusiness && cell.ownerId == ownerId) {
            ++count;
        }
    }

    return count;
}

int GameRules::nearestBusinessCellIdFrom(const GameState& state, int position)
{
    const int boardSize = state.board().size();

    if (boardSize <= 0) {
        return -1;
    }

    const int startPosition = normalizedBoardPosition(position, boardSize);

    if (startPosition < 0) {
        return -1;
    }

    for (int offset = 1; offset <= boardSize; ++offset) {
        const int cellId = (startPosition + offset) % boardSize;
        const Cell* cell = state.cellAt(cellId);

        if (cell != nullptr && isBusinessCell(*cell)) {
            return cellId;
        }
    }

    return -1;
}

int GameRules::propertyMaintenanceCost(const GameState& state, const Player& player, int costPerProperty)
{
    if (costPerProperty <= 0) {
        return 0;
    }

    int ownedBusinessCount = 0;
    int buildingLevelSum = 0;

    for (const Cell& cell : state.board()) {
        if (cell.ownerId != player.id) {
            continue;
        }

        ++ownedBusinessCount;
        buildingLevelSum += cell.buildingLevel;
    }

    return (ownedBusinessCount + buildingLevelSum) * costPerProperty;
}