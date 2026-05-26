#pragma once

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
    QFrame* LeftPanelWidget_;
    QFrame* CenterGameWidget_;
    QFrame* ChatWidget_;

    QHBoxLayout* rootLayout_;
};


