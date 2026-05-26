#include "players_list_widget.hpp"

PlayersListWidget::PlayersListWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("playersList");

    rootLayout_ = new QVBoxLayout(this);

    rootLayout_->setSpacing(12);
    rootLayout_->setContentsMargins(10,10,10,10);

    for (int i = 0; i < 6; ++i)
    {
        auto* playerCard = new PlayerCardGameWidget(this);

        playerCard->hide();

        playerCards_.push_back(playerCard);

        rootLayout_->addWidget(playerCard);
    }

    rootLayout_->addStretch();
}

PlayersListWidget::~PlayersListWidget() = default;

void PlayersListWidget::setPlayer(
    int index,
    const QString& nickname,
    qint64 balance
    )
{
    if (index < 0 || index >= playerCards_.size())
        return;

    auto* card = playerCards_[index];

    card->setNickname(nickname);
    card->setBalance(balance);

    card->show();
}

void PlayersListWidget::clearPlayer(int index)
{
    if (index < 0 || index >= playerCards_.size())
        return;

    playerCards_[index]->hide();
}