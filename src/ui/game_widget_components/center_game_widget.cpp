#include "game_widget_components/center_game_widget.hpp"

CenterGameWidget::CenterGameWidget(QWidget *parent)
    : QWidget(parent)
{
    boardWidget_ = new BoardWidget(this);

    rootLayout_ = new QVBoxLayout(this);
    rootLayout_->addWidget(boardWidget_);

    connect(boardWidget_, &BoardWidget::messageSent, this, &CenterGameWidget::messageSent);
    connect(boardWidget_, &BoardWidget::buildBusinessRequested, this, &CenterGameWidget::buildBusinessRequested);
}

CenterGameWidget::~CenterGameWidget() = default;

void CenterGameWidget::addEvent(const QString& event)
{
    boardWidget_->addEvent(event);
}

void CenterGameWidget::setPlayers(const QVector<ClientGamePlayer>& players)
{
    boardWidget_->setPlayers(players);
}

void CenterGameWidget::setCells(const QVector<ClientBoardCell>& cells)
{
    boardWidget_->setCells(cells);
}

void CenterGameWidget::setLocalPlayerId(quint16 playerId)
{
    boardWidget_->setLocalPlayerId(playerId);
}