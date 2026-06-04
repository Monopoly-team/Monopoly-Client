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

    void addPlayer(
        int playerId,
        const QString& nickname,
        const QString& color = QString(),
        const QString& avatarPath = QString()
        );
    bool removePlayer(int playerId);
    bool startGame();

    bool handlePlayerAction(int playerId, const QJsonObject& payload);

    QJsonObject gameStateToJson() const;
    QStringList takeEvents();

    bool buyAuctionBusiness(int playerId, int cellId, int price);
    void finishCurrentTurn();

    const GameState& gameState() const;

    bool applyAdminAction(
        const QString& action,
        int playerId,
        int amount,
        int balance,
        const QString& reason
        );

signals:
    void stateChanged();

private:
    QString actionTypeFromJson(const QJsonObject& payload) const;

    bool handleRollDiceAction(Player& player);
    bool handleBuyBusinessAction(Player& player);
    bool handleBuildBusinessAction(Player& player, const QJsonObject& payload);
    bool handleEndTurnAction(Player& player);

    void movePlayer(Player& player, int steps);
    bool movePlayerToCell(Player& player, int cellId, bool shouldCollectStartBonus);
    bool movePlayerToNearestBusiness(Player& player);

    void handleLandingCell(Player& player, Cell& cell);

    void handleCornerCell(Player& player, const Cell& cell);
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

    bool shouldWaitForPurchaseDecision(const Player& player) const;

private:
    GameSession session_;
    QStringList events_;

    bool        hasRolledThisTurn_ = false;
};