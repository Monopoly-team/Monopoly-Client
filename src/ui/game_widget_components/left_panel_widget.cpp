#include "game_widget_components/left_panel_widget.hpp"
#include <QRandomGenerator>
LeftPanelWidget::LeftPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    playersList_ = new PlayersListWidget();
    rootLayout_  = new QVBoxLayout(this);

    rootLayout_->addWidget(playersList_);
    playersList_->setPlayer(0,"Polad",1333);
    playersList_->setPlayer(1,"Poladjan",4324);
    playersList_->setPlayer(2,"Poladin",124324);
    playersList_->setPlayer(3,"Poladius",324234);
    playersList_->setPlayer(4,"Poladik",12123123345);
    playersList_->setPlayer(5,"Poladick",12345);
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