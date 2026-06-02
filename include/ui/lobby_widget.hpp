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
    void readyChanged(bool ready);
public slots:
    void updatePlayers(const QVector<ClientLobbyPlayer>& players);
    void updateCountdown(int secondsLeft);
    void cancelCountdown();
private slots:
    void onReadyChanged();
public:
    explicit LobbyWidget(QWidget* parent = nullptr);
    ~LobbyWidget() override;
    QPushButton *readyButton() const;

private:
    QPushButton*    readyButton_;
    QLabel*         countdownLabel_;
    QLabel*         lobbyTitle_;
    QGridLayout*    playersGrid_;
    QHBoxLayout*    bottomLayout_;

    QVector<PlayerCardLobbyWidget*> playerCards_;
    bool isReady_ = false;


};


