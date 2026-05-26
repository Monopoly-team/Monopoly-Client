#include "player_card_lobby_widget.hpp"

PlayerCardLobbyWidget::PlayerCardLobbyWidget(QWidget *parent)
    : PlayerCardBaseWidget{parent}
{
    readyStatus_= new QLabel("Не готов", this);

    readyStatus_->setObjectName("readyLabel");

    rootLayout_->addWidget(readyStatus_);
}

PlayerCardLobbyWidget::~PlayerCardLobbyWidget() = default;

void PlayerCardLobbyWidget::setReady(bool isReady)
{
    isReady ? readyStatus_->setText("Готов") : readyStatus_->setText("Не готов");
}

