#include "game_widget.hpp"

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    rootLayout_ = new QHBoxLayout(this);

    leftPanelWidget_    = new LeftPanelWidget(this);
    centerGameWidget_   = new CenterGameWidget(this);
    chatWidget_         = new ChatWidget(this);

    rootLayout_->addWidget(leftPanelWidget_,1);
    rootLayout_->addWidget(centerGameWidget_,4);
    rootLayout_->addWidget(chatWidget_,1 );
}
