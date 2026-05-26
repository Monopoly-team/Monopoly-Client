#pragma once

#include "game_widget_components/left_panel_widget.hpp"
#include "game_widget_components/center_game_widget.hpp"
#include "game_widget_components/chat_widget.hpp"

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>

class GameWidget : public QWidget
{
    Q_OBJECT
signals:

private slots:

public:
    explicit GameWidget(QWidget* parent = nullptr);
private:
    LeftPanelWidget* leftPanelWidget_;
    CenterGameWidget* centerGameWidget_;
    ChatWidget* chatWidget_;

    QHBoxLayout* rootLayout_;
};


