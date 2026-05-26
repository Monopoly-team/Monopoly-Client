#pragma once

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

class PlayerCardWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PlayerCardWidget(QWidget *parent = nullptr);
    ~PlayerCardWidget() override;

    void setNickname(const QString& nickname);
    void setReady(bool isReady);
private:
    QLabel*      avatar_;
    QLabel*      nickname_;
    QLabel*      readyStatus_;
    QHBoxLayout* rootLayout_;
};


