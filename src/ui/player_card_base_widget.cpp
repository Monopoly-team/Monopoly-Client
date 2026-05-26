#include "player_card_base_widget.hpp"

PlayerCardBaseWidget::PlayerCardBaseWidget(QWidget *parent)
    : QFrame{parent}
{
    setFixedHeight(120);
    setFixedWidth(520);
    rootLayout_ = new QHBoxLayout(this);
    setObjectName("playerCard");

    avatar_     = new QLabel("[]",       this);
    nickname_   = new QLabel("Полад",  this);

    nickname_->setObjectName("nicknameLabel");
    avatar_->setObjectName("avatar");
    avatar_->setScaledContents(true);

    QPixmap avatar(":/resources/img/user2.png");
    avatar_->setFixedSize(96,96);
    avatar_->setPixmap(avatar.scaled(96,96,Qt::KeepAspectRatio, Qt::SmoothTransformation));

    rootLayout_->addSpacing(11);
    rootLayout_->addWidget(avatar_);
    rootLayout_->addSpacing(30);
    rootLayout_->addWidget(nickname_,2);
}

PlayerCardBaseWidget::~PlayerCardBaseWidget() = default;

void PlayerCardBaseWidget::setNickname(const QString &nickname)
{
    nickname_->setText(nickname);
}

