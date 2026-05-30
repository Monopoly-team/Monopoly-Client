#pragma once

#include <QJsonObject>
#include <QString>

class GameState
{
public:
    GameState();

    QJsonObject toJson() const;
    static GameState fromJson(const QJsonObject& json);

private:
    QString gameStatus_;
    int currentPlayerId_ = 0;
};