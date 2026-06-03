#pragma once

#include <QJsonObject>
#include <QString>
#include <QVector>

#include "game/models/business_group.hpp"
#include "game/models/cell_type.hpp"

constexpr int BOARD_SIZE = 40;
constexpr int PLAYER_START_BALANCE = 1500;
constexpr int NO_OWNER_ID = 0;
constexpr int NO_WINNER_ID = 0;

struct Cell
{
    int id = 0;
    QString name;
    CellType type = CellType::Business;
    BusinessGroup group = BusinessGroup::None;

    int price = 0;
    QVector<int> rentLevels;
    int mortgageValue = 0;

    int buildingCost = 0;
    int buildingLevel = 0;
    int maxBuildingLevel = 0;

    int ownerId = NO_OWNER_ID;
};

struct Player
{
    int id = 0;
    QString nickname;
    QString color;

    int position = 0;
    int balance = PLAYER_START_BALANCE;

    bool isBankrupt = false;
    bool active = true;
    bool isInJail = false;
    int roundsInJail = 0;

    QVector<int> ownedProperties;
};

class GameState
{
public:
    GameState();

    QJsonObject toJson() const;
    static GameState fromJson(const QJsonObject& json);

    void addPlayer(int id, const QString& nickname, const QString& color = QString());

    const QVector<Cell>& board() const;
    QVector<Cell>& board();

    const QVector<Player>& players() const;
    QVector<Player>& players();

    QString gameStatus() const;
    void setGameStatus(const QString& gameStatus);

    int currentPlayerId() const;
    void setCurrentPlayerId(int playerId);

    int lastDiceValue() const;
    void setLastDiceValue(int value);

    bool isGameOver() const;
    void setGameOver(bool isGameOver);

    int winnerId() const;
    void setWinnerId(int id);

    Player* playerById(int id);
    const Player* playerById(int id) const;

    Cell* cellAt(int index);
    const Cell* cellAt(int index) const;

    int lastDiceFirst() const;
    int lastDiceSecond() const;

    void setLastDiceValues(int first, int second);

private:
    QString gameStatus_ = "waiting";
    int currentPlayerId_ = NO_OWNER_ID;
    int lastDiceValue_ = 0;

    QVector<Player> players_;
    QVector<Cell> board_;

    bool isGameOver_ = false;
    int winnerId_ = NO_WINNER_ID;

    int lastDiceFirst_  = 1;
    int lastDiceSecond_ = 1;
};
