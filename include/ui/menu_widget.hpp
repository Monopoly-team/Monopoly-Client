#pragma once

#include "center_card_widget.hpp"

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>

class MenuWidget : public CenterCardWidget
{
public:
    explicit MenuWidget(QWidget* parent = nullptr);
    ~MenuWidget() override;
private:
    QLabel*         menuLabel_;

    QLineEdit*      nameEdit_;
    QLineEdit*      ipEdit_;

    QPushButton*    createButton_;
    QPushButton*    joinButton_;

    QHBoxLayout*    buttonsLayout_;
};


