#pragma once

#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class DiceWidget : public QFrame
{
    Q_OBJECT

public:
    explicit DiceWidget(QWidget* parent = nullptr);
    ~DiceWidget() override;

    void setDiceValues(int first, int second);
    void setRollEnabled(bool enabled);
signals:
    void rollRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* firstDice_;
    QLabel* secondDice_;
    QLabel* hintLabel_;

    QVBoxLayout* rootLayout_;
    QHBoxLayout* diceLayout_;
};