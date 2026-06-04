#pragma once

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

class PlayerCardBaseWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PlayerCardBaseWidget(QWidget *parent = nullptr);
    ~PlayerCardBaseWidget() override;
    void setAvatarPath(const QString& avatarPath);
    void setNickname(const QString& nickname);
protected:
    QLabel*      avatar_;
    QLabel*      nickname_;
    QHBoxLayout* rootLayout_;
};


