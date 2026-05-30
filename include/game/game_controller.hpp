#pragma once

#include <QObject>
#include <QJsonObject>
#include <QStringList>

#include "game_session.hpp"

class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(QObject* parent = nullptr);

    bool handlePlayerAction(
        int playerId,
        const QJsonObject& action);

    QStringList takeEvents();

    const GameState& gameState() const;

signals:
    void stateChanged();

private:
    GameSession session_;
    QStringList events_;
};