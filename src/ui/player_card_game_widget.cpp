#include "player_card_game_widget.hpp"
#include <QLocale>
PlayerCardGameWidget::PlayerCardGameWidget(QWidget *parent)
    : PlayerCardBaseWidget{parent}
{
    setFixedSize(460,80);
    avatar_->setFixedSize(60,60);
    nickname_->setObjectName("nicknameGameLabel");
    balanceLabel_ = new QLabel("10000",this);
    balanceLabel_->setObjectName("balanceLabel");
    rootLayout_->addWidget(balanceLabel_);
}

PlayerCardGameWidget::~PlayerCardGameWidget() = default;

void PlayerCardGameWidget::setBalance(qint64 balance)
{
    balanceLabel_->setText(
         "💵 " + QLocale().toString(balance)
        );
}

