#include "menu_widget.hpp"

MenuWidget::MenuWidget(QWidget *parent)
{
    menuLabel_ = new QLabel("Меню", card_);
    menuLabel_->setObjectName("loginSubtitle");
    menuLabel_->setAlignment(Qt::AlignCenter);

    nameEdit_ = new QLineEdit(card_);
    ipEdit_   = new QLineEdit(card_);

    nameEdit_->setPlaceholderText("Имя");
    nameEdit_->setObjectName("loginEdit");
    nameEdit_->setFixedHeight(62);
    nameEdit_->setMaxLength(24);
    nameEdit_->setFrame(false);

    ipEdit_->setPlaceholderText("IP для подключения (например: 127.0.0.1)");
    ipEdit_->setObjectName("loginEdit");
    ipEdit_->setFixedHeight(62);
    ipEdit_->setMaxLength(24);
    ipEdit_->setFrame(false);

    buttonsLayout_ = new QHBoxLayout();

    createButton_  = new QPushButton("Создать игру",card_);
    joinButton_    = new QPushButton("Присоединиться к игре",card_);

    createButton_->setObjectName("loginButton");
    createButton_->setFixedHeight(82);
    createButton_->setFixedWidth(420);
    createButton_->setCursor(Qt::PointingHandCursor);

    joinButton_->setObjectName("loginButton");
    joinButton_->setFixedHeight(82);
    joinButton_->setFixedWidth(420);
    joinButton_->setCursor(Qt::PointingHandCursor);

    createButton_->setEnabled(false);
    joinButton_->setEnabled(false);

    buttonsLayout_->addWidget(createButton_);
    buttonsLayout_->addWidget(joinButton_);


    cardLayout_->addStretch();
    cardLayout_->addWidget(menuLabel_,0,Qt::AlignTop);
    cardLayout_->addSpacing(70);
    cardLayout_->addWidget(nameEdit_);
    cardLayout_->addWidget(ipEdit_);
    cardLayout_->addSpacing(70);
    cardLayout_->addLayout(buttonsLayout_);
    cardLayout_->addStretch();

    connect(nameEdit_, &QLineEdit::textChanged, this, [this]() {
        updateActionButtonsState();
    });
    connect(createButton_, &QPushButton::clicked, this, &MenuWidget::createGameRequested);
    connect(joinButton_,   &QPushButton::clicked, this, &MenuWidget::joinGameRequested);
}

MenuWidget::~MenuWidget() = default;

QString MenuWidget::serverIp() const
{
    return ipEdit_->text().trimmed();
}

QString MenuWidget::nickname() const
{
    return nameEdit_->text().trimmed();
}

QPushButton *MenuWidget::createButton() const
{
    return createButton_;
}

QPushButton *MenuWidget::joinButton() const
{
    return joinButton_;
}

bool MenuWidget::hasNickname() const
{
    return !nickname().isEmpty();
}

void MenuWidget::updateActionButtonsState()
{
    const bool canContinue = hasNickname();

    createButton_->setEnabled(canContinue);
    joinButton_->setEnabled(canContinue);

    createButton_->setCursor(canContinue ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
    joinButton_->setCursor(canContinue ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
}