#pragma once

#include <QFrame>
#include <QVBoxLayout>
#include <QVector>

#include "player_card_game_widget.hpp"
#include "network/client/tcp_client_controller.hpp"

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
    void updatePlayers(const QVector<ClientGamePlayer>& players, quint16 winnerId = 0);

private:
    QVBoxLayout* rootLayout_;

    QVector<PlayerCardGameWidget*> playerCards_;
};