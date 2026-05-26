#pragma once

#include "center_card_widget.hpp"

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class LobbyWidget : public CenterCardWidget
{
    Q_OBJECT
signals:
    void startRequested();
public:
    explicit LobbyWidget(QWidget* parent = nullptr);
    ~LobbyWidget() override;
private:
    QPushButton*    readyButton_;
    QLabel*         lobbyTitle_;
    QGridLayout*    playersGrid_;
};


