#pragma once

#include "player_card_base_widget.hpp"

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

class PlayerCardGameWidget : public PlayerCardBaseWidget
{
    Q_OBJECT
public:
    explicit PlayerCardGameWidget(QWidget *parent = nullptr);
    ~PlayerCardGameWidget() override;

    void setBalance(qint64 balance);
private:
    QLabel* balanceLabel_;
};


