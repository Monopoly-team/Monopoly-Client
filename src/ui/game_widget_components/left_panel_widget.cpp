#include "game_widget_components/left_panel_widget.hpp"

LeftPanelWidget::LeftPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    rootLayout_ = new QVBoxLayout(this);

    playersList_ = new PlayersListWidget(this);

    buyBusinessButton_ = new QPushButton("Купить бизнес", this);
    endTurnButton_ = new QPushButton("Завершить ход", this);

    diceWidget_ = new DiceWidget(this);

    rootLayout_->addWidget(playersList_, 2);
    rootLayout_->addWidget(diceWidget_, 1);
    rootLayout_->addWidget(buyBusinessButton_);
    rootLayout_->addWidget(endTurnButton_);

    connect(diceWidget_, &DiceWidget::rollRequested, this, &LeftPanelWidget::rollDiceRequested);
    connect(buyBusinessButton_, &QPushButton::clicked, this, &LeftPanelWidget::buyBusinessRequested);
    connect(endTurnButton_, &QPushButton::clicked, this, &LeftPanelWidget::endTurnRequested);
}

LeftPanelWidget::~LeftPanelWidget()= default;

void LeftPanelWidget::updatePlayers(const QVector<ClientGamePlayer>& players)
{
    playersList_->updatePlayers(players);
}

void LeftPanelWidget::setDiceValues(int first, int second)
{
    diceWidget_->setDiceValues(first, second);
}