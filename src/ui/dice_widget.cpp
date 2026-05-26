#include "dice_widget.hpp"

#include <QMouseEvent>
#include <QPixmap>

DiceWidget::DiceWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("diceWidget");

    setCursor(Qt::PointingHandCursor);

    firstDice_ = new QLabel(this);
    secondDice_ = new QLabel(this);

    firstDice_->setObjectName("diceLabel");
    secondDice_->setObjectName("diceLabel");

    firstDice_->setFixedSize(150,150);
    secondDice_->setFixedSize(150,150);

    firstDice_->setAlignment(Qt::AlignCenter);
    secondDice_->setAlignment(Qt::AlignCenter);

    hintLabel_ = new QLabel("Нажмите чтобы бросить", this);

    hintLabel_->setObjectName("diceHint");
    hintLabel_->setAlignment(Qt::AlignCenter);

    diceLayout_ = new QHBoxLayout();

    diceLayout_->setSpacing(20);
    diceLayout_->setAlignment(Qt::AlignCenter);

    diceLayout_->addWidget(firstDice_);
    diceLayout_->addWidget(secondDice_);

    rootLayout_ = new QVBoxLayout(this);

    rootLayout_->setContentsMargins(20,20,20,20);

    rootLayout_->addStretch();
    rootLayout_->addLayout(diceLayout_);
    rootLayout_->addSpacing(12);
    rootLayout_->addWidget(hintLabel_);
    rootLayout_->addStretch();

    setDiceValues(1,1);
}

DiceWidget::~DiceWidget() = default;

void DiceWidget::mousePressEvent(QMouseEvent* event)
{
    emit rollRequested();

    QFrame::mousePressEvent(event);
}

void DiceWidget::setDiceValues(int first, int second)
{
    QString firstPath =
        QString(":/resources/img/%1.png").arg(first);

    QString secondPath =
        QString(":/resources/img/%1.png").arg(second);

    QPixmap firstPixmap(firstPath);
    QPixmap secondPixmap(secondPath);

    firstDice_->setPixmap(
        firstPixmap.scaled(
            150,
            150,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            )
        );

    secondDice_->setPixmap(
        secondPixmap.scaled(
            150,
            150,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            )
        );
}