#include "game_widget.hpp"

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{

    rootLayout_ = new QHBoxLayout(this);

    leftPanelWidget_    = new LeftPanelWidget(this);
    centerGameWidget_   = new CenterGameWidget(this);

    rootLayout_->addWidget(leftPanelWidget_,1);
    rootLayout_->addWidget(centerGameWidget_,6);

    connect(centerGameWidget_, &CenterGameWidget::messageSent,      this, &GameWidget::messageSent);
    connect(leftPanelWidget_,  &LeftPanelWidget::rollDiceRequested, this, &GameWidget::rollDiceRequested);

}

GameWidget::~GameWidget() = default;

void GameWidget::addEvent(const QString& event)
{
    centerGameWidget_->addEvent(event);
}

void GameWidget::updatePlayers(const QVector<ClientGamePlayer>& players)
{
    leftPanelWidget_->updatePlayers(players);
    centerGameWidget_->setPlayers(players);
}

void GameWidget::updateGameState(const ClientGameState& state)
{
    leftPanelWidget_->updatePlayers(state.players);
    leftPanelWidget_->setDiceValues(state.lastDiceFirst, state.lastDiceSecond);

    centerGameWidget_->setPlayers(state.players);
    centerGameWidget_->setCells(state.cells);
}
