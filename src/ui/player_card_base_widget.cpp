#include "player_card_base_widget.hpp"

#include <QPixmap>

PlayerCardBaseWidget::PlayerCardBaseWidget(QWidget *parent)
    : QFrame{parent}
{
    setFixedHeight(120);
    setFixedWidth(520);
    rootLayout_ = new QHBoxLayout(this);
    setObjectName("playerCard");

    avatar_     = new QLabel("[]",       this);
    nickname_   = new QLabel("Игрок",  this);

    nickname_->setObjectName("nicknameLabel");
    avatar_->setObjectName("avatar");
    avatar_->setScaledContents(true);

    avatar_->setFixedSize(96, 96);
    setAvatarPath(QStringLiteral(":/resources/img/user2.png"));

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

void PlayerCardBaseWidget::setAvatarPath(const QString& avatarPath)
{
    const QString path = avatarPath.trimmed().isEmpty()
    ? QStringLiteral(":/resources/img/user2.png")
    : avatarPath;

    QPixmap avatarPixmap(path);

    if (avatarPixmap.isNull())
        avatarPixmap.load(QStringLiteral(":/resources/img/user2.png"));

    avatar_->setPixmap(
        avatarPixmap.scaled(
            avatar_->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            )
        );
}