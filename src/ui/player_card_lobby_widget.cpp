#include "player_card_lobby_widget.hpp"
#include <QStyle>
PlayerCardLobbyWidget::PlayerCardLobbyWidget(QWidget *parent)
    : PlayerCardBaseWidget{parent}
{
    readyStatus_= new QLabel("Не готов", this);
    readyStatus_->setFixedHeight(40);
    readyStatus_->setAlignment(Qt::AlignCenter);
    readyStatus_->setObjectName("readyLabel");

    rootLayout_->addWidget(readyStatus_);
}

PlayerCardLobbyWidget::~PlayerCardLobbyWidget() = default;

void PlayerCardLobbyWidget::setReady(bool isReady)
{
    isReady ? readyStatus_->setText("Готов") : readyStatus_->setText("Не готов");
    readyStatus_->setProperty("ready", isReady);

    readyStatus_->style()->unpolish(readyStatus_);
    readyStatus_->style()->polish(readyStatus_);

}

