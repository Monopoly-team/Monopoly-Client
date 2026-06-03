#include "player_card_game_widget.hpp"
#include "ui/player_visuals.hpp"

#include <QLocale>

PlayerCardGameWidget::PlayerCardGameWidget(QWidget* parent)
    : PlayerCardBaseWidget{parent}
{
    setFixedSize(460, 80);

    avatar_->setFixedSize(60, 60);

    nickname_->setObjectName("nicknameGameLabel");

    balanceLabel_ = new QLabel("10000", this);
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

void PlayerCardGameWidget::setPlayer(const ClientGamePlayer& player, quint16 winnerId)
{
    setNickname(player.nickname);
    setBalance(player.balance);
    applyVisualState(player, winnerId);
}

void PlayerCardGameWidget::applyVisualState(const ClientGamePlayer& player, quint16 winnerId)
{
    const QColor color = PlayerVisuals::displayColor(player, winnerId);
    const QString colorName = color.name();

    setStyleSheet(
        QStringLiteral(
            "QFrame#playerCard {"
            "    background-color: rgba(0,0,0,0.20);"
            "    border-left: 8px solid %1;"
            "    border-radius: 18px;"
            "}"
            ).arg(colorName)
        );

    nickname_->setStyleSheet(
        QStringLiteral(
            "color: %1;"
            "font-size: 27px;"
            "font-weight: bold;"
            "background: transparent;"
            "font-family: \"Segoe UI\";"
            ).arg(colorName)
        );
    setToolTip(PlayerVisuals::statusText(player, winnerId));
}