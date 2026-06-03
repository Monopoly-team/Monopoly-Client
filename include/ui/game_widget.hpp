#pragma once

#include "game_widget_components/left_panel_widget.hpp"
#include "game_widget_components/center_game_widget.hpp"
#include "network/client/tcp_client_controller.hpp"

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>

class GameWidget : public QWidget
{
    Q_OBJECT
signals:
    void messageSent(const QString& message);
    void rollDiceRequested();
    void buildBusinessRequested(int cellId);
private slots:

public:
    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget() override;

    void addEvent(const QString& event);
    void updatePlayers(const QVector<ClientGamePlayer>& players);
    void updateGameState(const ClientGameState& state);
    void setLocalPlayerId(quint16 playerId);
    void setAuctionActive(bool active);
    void addChatMessage(quint16 playerId, const QString& nickname, const QString& text);
private:
    LeftPanelWidget* leftPanelWidget_;
    CenterGameWidget* centerGameWidget_;

    QHBoxLayout* rootLayout_;
    quint16      localPlayerId_ = 0;
    bool         auctionActive_ = false;
};


