#include "lobby_widget.hpp"

#include <algorithm>

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

    for (int i = 0; i < 6; ++i)
    {
        auto* playerCard = new PlayerCardLobbyWidget(card_);

        int row = i / 2;
        int col = i % 2;

        playersGrid_->addWidget(playerCard, row, col);

        playerCard->hide();

        playerCards_.push_back(playerCard);
    }
    qDebug() << "[Lobby] cards created:" << playerCards_.size();

    cardLayout_->addWidget(lobbyTitle_,0,Qt::AlignHCenter);
    cardLayout_->addSpacing(20);
    cardLayout_->addLayout(playersGrid_);
    cardLayout_->addWidget(readyButton_,0,Qt::AlignRight);

    connect(readyButton_, &QPushButton::clicked, this, &LobbyWidget::startRequested);
}

LobbyWidget::~LobbyWidget() = default;

void LobbyWidget::updatePlayers(const QVector<ClientLobbyPlayer> &players)
{
    qDebug() << "[Lobby] updatePlayers called";
    for(PlayerCardLobbyWidget* card : playerCards_)
        card->hide();

    const int count = std::min(players.size(), playerCards_.size());

    for(int i = 0; i < count; ++i)
    {
        playerCards_[i]->setNickname(players[i].nickname);
        playerCards_[i]->setReady(players[i].ready);
        playerCards_[i]->show();
    }
}