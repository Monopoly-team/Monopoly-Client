#include "game_widget_components/center_game_widget.hpp"

CenterGameWidget::CenterGameWidget(QWidget *parent)
    : QWidget(parent)
{
    boardWidget_ = new BoardWidget(this);

    rootLayout_ = new QVBoxLayout(this);
    rootLayout_->addWidget(boardWidget_);

    connect(boardWidget_, &BoardWidget::messageSent, this, &CenterGameWidget::messageSent);
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
