#pragma once

#include "center_card_widget.hpp"

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class LoginWidget : public CenterCardWidget
{
    Q_OBJECT
signals:
    void loginRequested();
public:
    explicit LoginWidget(QWidget* parent = nullptr);
    ~LoginWidget() override;
private:
    QLabel*      subtitleLabel_;

    QLineEdit*   loginEdit_;
    QLineEdit*   passwordEdit_;

    QPushButton* loginButton_;
};


