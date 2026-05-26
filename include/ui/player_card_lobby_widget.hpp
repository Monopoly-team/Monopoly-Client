#pragma once

#include "player_card_base_widget.hpp"

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

class PlayerCardLobbyWidget : public PlayerCardBaseWidget
{
    Q_OBJECT
public:
    explicit PlayerCardLobbyWidget(QWidget *parent = nullptr);
    ~PlayerCardLobbyWidget() override;

    void setReady(bool isReady);
private:
    QLabel*      readyStatus_;
};


