#pragma once

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class LoginWidget : public QWidget
{
public:
    explicit LoginWidget(QWidget* parent = nullptr);
    ~LoginWidget() override;
private:
    QFrame*      card_;
    QLabel*      titleLabel_;
    QLabel*      subtitleLabel_;
    QLineEdit*   nameEdit_;
    QLineEdit*   passwordEdit_;
    QPushButton* loginButton_;

    QVBoxLayout* rootLayout_;
    QVBoxLayout* cardLayout_;
};


