#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

#include "game/game_session.hpp"

class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(QObject* parent = nullptr);

    void addPlayer(int playerId, const QString& nickname);
    bool startGame();

    bool handlePlayerAction(int playerId, const QJsonObject& action);

    QStringList takeEvents();

    const GameState& gameState() const;

signals:
    void stateChanged();

private:
    QString actionTypeFromJson(const QJsonObject& action) const;

    bool handleRollDiceAction(Player& player, const QJsonObject& action);
    bool handleBuyBusinessAction(Player& player);
    bool handleBuildBusinessAction(Player& player, const QJsonObject& action);
    bool handleEndTurnAction(Player& player);

    void movePlayer(Player& player, int steps);
    bool movePlayerToCell(Player& player, int cellId, bool shouldCollectStartBonus);
    bool movePlayerToNearestBusiness(Player& player);

    void handleLandingCell(Player& player, Cell& cell);

    void handleStartCell(Player& player);
    void handleGoToJailCell(Player& player);
    void handleChanceCell(Player& player);
    void handleCommunityChestCell(Player& player);
    void handleBusinessCell(Player& player, Cell& cell);

    bool buyBusiness(Player& player, Cell& cell);
    bool buildBusiness(Player& player, Cell& cell);

    bool payRent(Player& player, Player& owner, const Cell& cell);
    bool payToBank(Player& player, int amount, const QString& reason);

    void giveMoney(Player& player, int amount, const QString& reason);
    void collectFromOtherPlayers(Player& player, int amountPerPlayer, const QString& reason);
    void sendPlayerToJail(Player& player);

    bool isCurrentPlayersTurn(int playerId) const;

    void finishTurn();
    void updateGameOver();

    void declareBankrupt(Player& player);
    void releaseOwnedProperties(Player& player);

    QString playerName(const Player& player) const;
    void appendEvent(const QString& eventText);

private:
    GameSession session_;
    QStringList events_;

    bool hasRolledThisTurn_ = false;
};