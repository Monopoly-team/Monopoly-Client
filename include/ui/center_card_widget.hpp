#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

class CenterCardWidget : public QWidget
{
public:
    explicit CenterCardWidget(QWidget* parent = nullptr);
protected:
    QVBoxLayout* rootLayout_;
    QVBoxLayout* cardLayout_;
    QLabel*      monopolyTitle_;
    QFrame*      card_;
};

inline CenterCardWidget::CenterCardWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("centerCardWidget");

    rootLayout_ = new QVBoxLayout(this);

    monopolyTitle_ = new QLabel("TCP Monopoly", this);
    monopolyTitle_->setObjectName("loginTitle");
    monopolyTitle_->setAlignment(Qt::AlignCenter);

    card_ = new QFrame(this);
    card_->setObjectName("loginCard");
    card_->setMinimumSize(800, 440);

    cardLayout_ = new QVBoxLayout(card_);
    cardLayout_->setSpacing(15);
    cardLayout_->setContentsMargins(30, 25, 30, 25);

    rootLayout_->addStretch();
    rootLayout_->addWidget(monopolyTitle_, 0, Qt::AlignHCenter);
    rootLayout_->addSpacing(20);
    rootLayout_->addWidget(card_, 0, Qt::AlignHCenter);
    rootLayout_->addStretch();
}