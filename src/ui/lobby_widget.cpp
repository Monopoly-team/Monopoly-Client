#include "lobby_widget.hpp"
#include "player_card_widget.hpp"
#include <QVector>

LobbyWidget::LobbyWidget(QWidget *parent)
    : CenterCardWidget(parent)
{
    card_->setMinimumSize(1200,600);

    lobbyTitle_  = new QLabel("Лобби",card_);
    lobbyTitle_->setAlignment(Qt::AlignTop);
    lobbyTitle_->setObjectName("loginSubtitle");

    readyButton_ = new QPushButton("Готов",card_);
    readyButton_->setObjectName("readyButton");
    readyButton_->setFixedSize(420,150);

    playersGrid_ = new QGridLayout();
    playersGrid_->setHorizontalSpacing(20);
    playersGrid_->setVerticalSpacing(20);
    playersGrid_->setRowStretch(0, 1);
    playersGrid_->setRowStretch(1, 1);
    playersGrid_->setRowStretch(2, 1);
    playersGrid_->setColumnStretch(0, 0);
    playersGrid_->setColumnStretch(1, 0);

    QVector<PlayerCardWidget*> playerCards_;

    for (int i = 0; i < 6; ++i)
    {
        auto* playerCard = new PlayerCardWidget(card_);

        int row = i / 2;
        int col = i % 2;

        playersGrid_->addWidget(playerCard, row, col);

        playerCard->hide();

        playerCards_.push_back(playerCard);
    }
    playerCards_[0]->show();
    playerCards_[1]->show();
    playerCards_[2]->show();
    playerCards_[3]->show();
    playerCards_[4]->show();
    playerCards_[5]->show();

    cardLayout_->addWidget(lobbyTitle_,0,Qt::AlignHCenter);
    cardLayout_->addSpacing(20);
    cardLayout_->addLayout(playersGrid_);
    cardLayout_->addWidget(readyButton_,0,Qt::AlignRight);

    connect(readyButton_, &QPushButton::clicked, this, &LobbyWidget::startRequested);
}

LobbyWidget::~LobbyWidget() = default;