#include "player_card_game_widget.hpp"

PlayerCardGameWidget::PlayerCardGameWidget(QWidget *parent)
    : PlayerCardBaseWidget{parent}
{
    balanceLabel_ = new QLabel("10000",this);
    rootLayout_->addWidget(balanceLabel_);
}

PlayerCardGameWidget::~PlayerCardGameWidget() = default;

void PlayerCardGameWidget::setBalance(qint64 balance)
{
    balanceLabel_->setText(QString::number(balance));
}

