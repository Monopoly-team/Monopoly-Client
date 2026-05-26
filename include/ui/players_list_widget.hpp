#pragma once

#include <QFrame>
#include <QVBoxLayout>
#include <QVector>

#include "player_card_game_widget.hpp"

class PlayersListWidget : public QFrame
{
    Q_OBJECT

public:
    explicit PlayersListWidget(QWidget* parent = nullptr);
    ~PlayersListWidget() override;

    void setPlayer(
        int index,
        const QString& nickname,
        qint64 balance
        );

    void clearPlayer(int index);

private:
    QVBoxLayout* rootLayout_;

    QVector<PlayerCardGameWidget*> playerCards_;
};