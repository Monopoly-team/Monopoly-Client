#include "game_widget_components/center_game_widget.hpp"

CenterGameWidget::CenterGameWidget(QWidget *parent)
    : QWidget(parent)
{
    boardWidget_ = new BoardWidget(this);

    rootLayout_ = new QVBoxLayout(this);
    rootLayout_->addWidget(boardWidget_);
}

CenterGameWidget::~CenterGameWidget() = default;
