#pragma once

#include "player_card_base_widget.hpp"
#include "network/client/tcp_client_controller.hpp"

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

class PlayerCardGameWidget : public PlayerCardBaseWidget
{
    Q_OBJECT

public:
    explicit PlayerCardGameWidget(QWidget* parent = nullptr);
    ~PlayerCardGameWidget() override;

    void setBalance(qint64 balance);
    void setPlayer(const ClientGamePlayer& player, quint16 winnerId);

private:
    void applyVisualState(const ClientGamePlayer& player, quint16 winnerId);

private:
    QLabel* balanceLabel_;
};