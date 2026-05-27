#pragma once
#include <QWidget>
#include <QVBoxLayout>

#include "board_widget.hpp"

class CenterGameWidget : public QWidget
{
public:
    explicit CenterGameWidget(QWidget* parent = nullptr);
    ~CenterGameWidget() override;
private:
    BoardWidget* boardWidget_;

    QVBoxLayout* rootLayout_;
};
