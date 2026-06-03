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
private slots:

public:
    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget() override;

    void addEvent(const QString& event);
    void updatePlayers(const QVector<ClientGamePlayer>& players);
    void updateGameState(const ClientGameState& state);
private:
    LeftPanelWidget* leftPanelWidget_;
    CenterGameWidget* centerGameWidget_;

    QHBoxLayout* rootLayout_;
};


