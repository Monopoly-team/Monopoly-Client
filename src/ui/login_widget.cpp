#include "login_widget.hpp"

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(0,0,0,0);
    rootLayout_ = new QVBoxLayout(this);

    card_ = new QFrame(this);
    card_->setFixedSize(800, 440);
    card_->setObjectName("loginCard");

    titleLabel_ = new QLabel("TCP Monopoly", card_);
    titleLabel_->setObjectName("loginTitle");
    titleLabel_->setAlignment(Qt::AlignCenter);

    nameEdit_ = new QLineEdit(card_);
    passwordEdit_ = new QLineEdit(card_);

    nameEdit_->setPlaceholderText("Имя");
    nameEdit_->setObjectName("loginEdit");
    nameEdit_->setFixedHeight(52);
    nameEdit_->setMaxLength(24);
    nameEdit_->setFrame(false);

    passwordEdit_->setFrame(false);
    passwordEdit_->setMaxLength(32);
    passwordEdit_->setPlaceholderText("Пароль");
    passwordEdit_->setObjectName("loginEdit");
    passwordEdit_->setFixedHeight(52);
    passwordEdit_->setEchoMode(QLineEdit::Password);

    loginButton_ = new QPushButton("Войти",card_);
    loginButton_->setObjectName("loginButton");
    loginButton_->setFixedHeight(52);
    loginButton_->setFixedWidth(420);
    loginButton_->setCursor(Qt::PointingHandCursor);




    cardLayout_ = new QVBoxLayout(card_);
    cardLayout_->setSpacing(15);
    cardLayout_->setContentsMargins(30, 25, 30, 25);

    subtitleLabel_ = new QLabel("Вход", card_);
    subtitleLabel_->setObjectName("loginSubtitle");
    subtitleLabel_->setAlignment(Qt::AlignCenter);

    cardLayout_->addWidget(subtitleLabel_,0, Qt::AlignTop);
    cardLayout_->addWidget(nameEdit_);
    cardLayout_->addWidget(passwordEdit_);
    cardLayout_->addSpacing(70);
    cardLayout_->addWidget(loginButton_,0, Qt::AlignCenter);

    rootLayout_->addStretch();
    rootLayout_->addWidget(titleLabel_, 0, Qt::AlignHCenter);
    rootLayout_->addSpacing(20);
    rootLayout_->addWidget(card_, 0, Qt::AlignHCenter);
    rootLayout_->addStretch();
}

LoginWidget::~LoginWidget() = default;