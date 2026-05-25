#include "login_widget.hpp"

LoginWidget::LoginWidget(QWidget *parent)
    : CenterCardWidget(parent)
{
    loginEdit_ = new QLineEdit(card_);
    passwordEdit_ = new QLineEdit(card_);

    loginEdit_->setPlaceholderText("Имя");
    loginEdit_->setObjectName("loginEdit");
    loginEdit_->setFixedHeight(62);
    loginEdit_->setMaxLength(24);
    loginEdit_->setFrame(false);

    passwordEdit_->setFrame(false);
    passwordEdit_->setMaxLength(32);
    passwordEdit_->setPlaceholderText("Пароль");
    passwordEdit_->setObjectName("loginEdit");
    passwordEdit_->setFixedHeight(62);
    passwordEdit_->setEchoMode(QLineEdit::Password);

    loginButton_ = new QPushButton("Войти", card_);
    loginButton_->setObjectName("loginButton");
    loginButton_->setFixedHeight(52);
    loginButton_->setFixedWidth(420);
    loginButton_->setCursor(Qt::PointingHandCursor);

    subtitleLabel_ = new QLabel("Вход", card_);
    subtitleLabel_->setObjectName("loginSubtitle");
    subtitleLabel_->setAlignment(Qt::AlignCenter);

    cardLayout_->addWidget(subtitleLabel_, 0, Qt::AlignTop);
    cardLayout_->addWidget(loginEdit_);
    cardLayout_->addWidget(passwordEdit_);

    cardLayout_->addSpacing(70);

    cardLayout_->addWidget(loginButton_, 0, Qt::AlignCenter);

    connect(loginButton_, &QPushButton::clicked, this, &LoginWidget::loginRequested);
}


LoginWidget::~LoginWidget() = default;