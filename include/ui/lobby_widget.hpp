#pragma once

#include "center_card_widget.hpp"
#include "network/client/tcp_client_controller.hpp"
#include "player_card_lobby_widget.hpp"

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class LobbyWidget : public CenterCardWidget
{
    Q_OBJECT
signals:
    void startRequested();
public slots:
    void updatePlayers(const QVector<ClientLobbyPlayer>& players);
public:
    explicit LobbyWidget(QWidget* parent = nullptr);
    ~LobbyWidget() override;
private:
    QPushButton*    readyButton_;
    QLabel*         lobbyTitle_;
    QGridLayout*    playersGrid_;

    QVector<PlayerCardLobbyWidget*> playerCards_;
};


