#pragma once

#include "players_list_widget.hpp"
#include "dice_widget.hpp"
#include "network/client/tcp_client_controller.hpp"

#include <QWidget>
#include <QVBoxLayout>

class LeftPanelWidget : public QWidget
{
    Q_OBJECT

signals:
    void rollDiceRequested();
public:
    explicit LeftPanelWidget(QWidget* parent = nullptr);
    ~LeftPanelWidget() override;

    void updatePlayers(const QVector<ClientGamePlayer>& players, quint16 winnerId = 0);
    void setDiceValues(int first, int second);
    void setRollEnabled(bool enabled);

private:
    QVBoxLayout*        rootLayout_;
    PlayersListWidget*  playersList_;
    DiceWidget*         diceWidget_;
};
