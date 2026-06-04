#include "game/game_session.hpp"

namespace {

const QString WAITING_STATUS = QStringLiteral("waiting");
const QString PLAYING_STATUS = QStringLiteral("playing");
const QString FINISHED_STATUS = QStringLiteral("finished");

} // namespace

GameSession::GameSession() = default;

GameState& GameSession::state()
{
    return state_;
}

const GameState& GameSession::state() const
{
    return state_;
}

void GameSession::reset()
{
    state_ = GameState();
}

bool GameSession::addPlayer(
    int playerId,
    const QString& nickname,
    const QString& color,
    const QString& avatarPath
    )
{
    if (isPlaying() || state_.isGameOver()) {
        return false;
    }

    if (Player* player = state_.playerById(playerId)) {
        player->nickname = nickname;
        if (!avatarPath.trimmed().isEmpty()) {
            player->avatarPath = avatarPath;
        }
        if (!color.trimmed().isEmpty()) {
            player->color = color;
        }
        return true;
    }

    state_.addPlayer(playerId, nickname, color, avatarPath);

    if (state_.currentPlayerId() == NO_OWNER_ID) {
        state_.setCurrentPlayerId(playerId);
    }

    return true;
}

bool GameSession::removePlayer(int playerId)
{
    const int playerIndex = playerIndexById(playerId);

    if (playerIndex < 0) {
        return false;
    }

    QVector<Player>& players = state_.players();
    const bool wasCurrentPlayer = state_.currentPlayerId() == playerId;

    if (isPlaying()) {
        releasePlayerProperties(players[playerIndex]);

        players[playerIndex].active = false;
        players[playerIndex].isInJail = false;

        if (activePlayersCount() <= 1) {
            const Player* winner = firstActivePlayer();
            finish(winner != nullptr ? winner->id : NO_WINNER_ID);
            return true;
        }

        if (wasCurrentPlayer) {
            Player* nextPlayer = firstActivePlayer();
            state_.setCurrentPlayerId(nextPlayer != nullptr ? nextPlayer->id : NO_OWNER_ID);
        }

        return true;
    }

    releasePlayerProperties(players[playerIndex]);
    players.removeAt(playerIndex);

    if (players.isEmpty()) {
        state_.setCurrentPlayerId(NO_OWNER_ID);
        return true;
    }

    if (wasCurrentPlayer) {
        Player* nextPlayer = firstActivePlayer();
        state_.setCurrentPlayerId(nextPlayer != nullptr ? nextPlayer->id : NO_OWNER_ID);
    }

    return true;
}

bool GameSession::start()
{
    if (!isWaiting()) {
        return false;
    }

    if (state_.players().isEmpty()) {
        return false;
    }

    Player* firstPlayer = firstActivePlayer();

    if (firstPlayer == nullptr) {
        return false;
    }

    state_.setGameStatus(PLAYING_STATUS);
    state_.setCurrentPlayerId(firstPlayer->id);
    state_.setGameOver(false);
    state_.setWinnerId(NO_WINNER_ID);

    return true;
}

void GameSession::finish(int winnerId)
{
    state_.setGameStatus(FINISHED_STATUS);
    state_.setGameOver(true);
    state_.setWinnerId(winnerId);

    if (winnerId != NO_WINNER_ID) {
        state_.setCurrentPlayerId(winnerId);
    }
}

bool GameSession::isWaiting() const
{
    return state_.gameStatus() == WAITING_STATUS;
}

bool GameSession::isPlaying() const
{
    return state_.gameStatus() == PLAYING_STATUS;
}

bool GameSession::isFinished() const
{
    return state_.gameStatus() == FINISHED_STATUS;
}

bool GameSession::hasPlayer(int playerId) const
{
    return state_.playerById(playerId) != nullptr;
}

Player* GameSession::currentPlayer()
{
    return state_.playerById(state_.currentPlayerId());
}

const Player* GameSession::currentPlayer() const
{
    return state_.playerById(state_.currentPlayerId());
}

Player* GameSession::firstActivePlayer()
{
    for (Player& player : state_.players()) {
        if (!player.isBankrupt && player.active) {
            return &player;
        }
    }

    return nullptr;
}

const Player* GameSession::firstActivePlayer() const
{
    for (const Player& player : state_.players()) {
        if (!player.isBankrupt && player.active) {
            return &player;
        }
    }

    return nullptr;
}

Player* GameSession::nextActivePlayerAfter(int playerId)
{
    QVector<Player>& players = state_.players();

    if (players.isEmpty()) {
        return nullptr;
    }

    const int currentIndex = playerIndexById(playerId);

    if (currentIndex < 0) {
        return firstActivePlayer();
    }

    for (int offset = 1; offset <= players.size(); ++offset) {
        const int nextIndex = (currentIndex + offset) % players.size();

        if (!players[nextIndex].isBankrupt && players[nextIndex].active) {
            return &players[nextIndex];
        }
    }

    return nullptr;
}

const Player* GameSession::nextActivePlayerAfter(int playerId) const
{
    const QVector<Player>& players = state_.players();

    if (players.isEmpty()) {
        return nullptr;
    }

    const int currentIndex = playerIndexById(playerId);

    if (currentIndex < 0) {
        return firstActivePlayer();
    }

    for (int offset = 1; offset <= players.size(); ++offset) {
        const int nextIndex = (currentIndex + offset) % players.size();

        if (!players[nextIndex].isBankrupt && players[nextIndex].active) {
            return &players[nextIndex];
        }
    }

    return nullptr;
}

int GameSession::activePlayersCount() const
{
    int count = 0;

    for (const Player& player : state_.players()) {
        if (!player.isBankrupt && player.active) {
            ++count;
        }
    }

    return count;
}

int GameSession::playerIndexById(int playerId) const
{
    const QVector<Player>& players = state_.players();

    for (int i = 0; i < players.size(); ++i) {
        if (players[i].id == playerId) {
            return i;
        }
    }

    return -1;
}

void GameSession::releasePlayerProperties(Player& player)
{
    for (Cell& cell : state_.board()) {
        if (cell.ownerId != player.id) {
            continue;
        }

        cell.ownerId = NO_OWNER_ID;
        cell.buildingLevel = 0;
    }

    player.ownedProperties.clear();
}