#include "game_widget_components/left_panel_widget.hpp"

#include <QRandomGenerator>
LeftPanelWidget::LeftPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    playersList_ = new PlayersListWidget();
    rootLayout_  = new QVBoxLayout(this);

    rootLayout_->addWidget(playersList_);
    playersList_ = new PlayersListWidget(this);
    diceWidget_ = new DiceWidget(this);

    rootLayout_->addWidget(playersList_, 2);
    rootLayout_->addWidget(diceWidget_, 1);
    connect(diceWidget_, &DiceWidget::rollRequested, this, [this]() {
        diceWidget_->setDiceValues(
            QRandomGenerator::global()->bounded(1, 7),
            QRandomGenerator::global()->bounded(1, 7)
            );
    });
}

LeftPanelWidget::~LeftPanelWidget()= default;

void LeftPanelWidget::updatePlayers(const QVector<ClientGamePlayer>& players)
{
    playersList_->updatePlayers(players);
}