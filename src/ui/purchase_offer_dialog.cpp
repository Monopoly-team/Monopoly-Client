#include "ui/purchase_offer_dialog.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

PurchaseOfferDialog::PurchaseOfferDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("purchaseOfferDialog");
    setModal(true);
    setWindowTitle("Покупка бизнеса");
    setFixedSize(420, 230);

    titleLabel_ = new QLabel(this);
    titleLabel_->setObjectName("purchaseTitle");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(true);

    priceLabel_ = new QLabel(this);
    priceLabel_->setObjectName("purchasePrice");
    priceLabel_->setAlignment(Qt::AlignCenter);

    buyButton_ = new QPushButton("Купить", this);
    buyButton_->setObjectName("primaryDialogButton");

    auctionButton_ = new QPushButton("Торги", this);
    auctionButton_->setObjectName("secondaryDialogButton");

    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(buyButton_);
    buttonsLayout->addWidget(auctionButton_);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(18);
    rootLayout->addWidget(titleLabel_);
    rootLayout->addWidget(priceLabel_);
    rootLayout->addStretch();
    rootLayout->addLayout(buttonsLayout);

    connect(buyButton_, &QPushButton::clicked, this, [this]() {
        emit buyRequested();
        accept();
    });

    connect(auctionButton_, &QPushButton::clicked, this, [this]() {
        emit auctionRequested();
        accept();
    });
}

void PurchaseOfferDialog::setOffer(int cellId, const QString& cellName, int price)
{
    cellId_ = cellId;

    titleLabel_->setText(QString("Купить бизнес «%1»?").arg(cellName));
    priceLabel_->setText(QString("Цена: %1 $").arg(price));
}