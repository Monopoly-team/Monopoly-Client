#pragma once

#include "players_list_widget.hpp"
#include "dice_widget.hpp"

#include <QWidget>
#include <QVBoxLayout>

class LeftPanelWidget : public QWidget
{
public:
    explicit LeftPanelWidget(QWidget* parent = nullptr);
    ~LeftPanelWidget() override;
private:
    QVBoxLayout*        rootLayout_;
    PlayersListWidget*  playersList_;
    DiceWidget*         diceWidget_;
};
