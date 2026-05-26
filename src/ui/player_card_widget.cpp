#include "player_card_widget.hpp"

PlayerCardWidget::PlayerCardWidget(QWidget *parent)
    : QFrame{parent}
{
    setFixedHeight(120);
    setFixedWidth(520);
    rootLayout_ = new QHBoxLayout(this);
    setObjectName("playerCard");

    avatar_     = new QLabel("[]",       this);
    nickname_   = new QLabel("Полад",  this);
    readyStatus_= new QLabel("Не готов", this);

    nickname_->setObjectName("nicknameLabel");
    readyStatus_->setObjectName("readyLabel");
    avatar_->setObjectName("avatar");
    avatar_->setScaledContents(true);

    QPixmap avatar(":/resources/img/user2.png");
    avatar_->setFixedSize(96,96);
    avatar_->setPixmap(avatar.scaled(96,96,Qt::KeepAspectRatio, Qt::SmoothTransformation));

    rootLayout_->addWidget(avatar_);
    rootLayout_->addSpacing(30);
    rootLayout_->addWidget(nickname_,2);
    rootLayout_->addWidget(readyStatus_);
}

PlayerCardWidget::~PlayerCardWidget() = default;

void PlayerCardWidget::setNickname(const QString &nickname)
{
    nickname_->setText(nickname);
}

void PlayerCardWidget::setReady(bool isReady)
{
    isReady ? readyStatus_->setText("Готов") : readyStatus_->setText("Не готов");
}

