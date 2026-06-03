#pragma once

#include <QString>

#include "game/game_state.hpp"

class GameSession
{
public:
    GameSession();

    GameState& state();
    const GameState& state() const;

    void reset();

    bool addPlayer(int playerId, const QString& nickname, const QString& color = QString());
    bool removePlayer(int playerId);

    bool start();
    void finish(int winnerId = NO_WINNER_ID);

    bool isWaiting() const;
    bool isPlaying() const;
    bool isFinished() const;

    bool hasPlayer(int playerId) const;

    Player* currentPlayer();
    const Player* currentPlayer() const;

    Player* firstActivePlayer();
    const Player* firstActivePlayer() const;

    Player* nextActivePlayerAfter(int playerId);
    const Player* nextActivePlayerAfter(int playerId) const;

    int activePlayersCount() const;

private:
    int playerIndexById(int playerId) const;
    void releasePlayerProperties(Player& player);

private:
    GameState state_;
};