#include "ui/auction_dialog.hpp"

#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>

AuctionDialog::AuctionDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("auctionDialog");
    setModal(false);
    setWindowTitle("Торги");
    setFixedSize(460, 280);

    titleLabel_ = new QLabel(this);
    titleLabel_->setObjectName("auctionTitle");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(true);

    timerLabel_ = new QLabel(this);
    timerLabel_->setObjectName("auctionTimer");
    timerLabel_->setAlignment(Qt::AlignCenter);

    currentBidLabel_ = new QLabel(this);
    currentBidLabel_->setObjectName("auctionBidLabel");
    currentBidLabel_->setAlignment(Qt::AlignCenter);

    bidSpinBox_ = new QSpinBox(this);
    bidSpinBox_->setObjectName("auctionBidInput");
    bidSpinBox_->setRange(1, 999999);
    bidSpinBox_->setSingleStep(10);

    bidButton_ = new QPushButton("Сделать ставку", this);
    bidButton_->setObjectName("primaryDialogButton");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(14);
    rootLayout->addWidget(titleLabel_);
    rootLayout->addWidget(timerLabel_);
    rootLayout->addWidget(currentBidLabel_);
    rootLayout->addWidget(bidSpinBox_);
    rootLayout->addWidget(bidButton_);

    connect(bidButton_, &QPushButton::clicked, this, [this]() {
        emit bidRequested(bidSpinBox_->value());
    });
}

void AuctionDialog::updateAuction(
    int cellId,
    const QString& cellName,
    int secondsLeft,
    int currentBid,
    const QString& highestBidderName
    )
{
    cellId_ = cellId;

    titleLabel_->setText(QString("Торги за «%1»").arg(cellName));
    timerLabel_->setText(QString("Осталось: %1 сек.").arg(secondsLeft));

    const QString bidder = highestBidderName.isEmpty()
                               ? "ставок пока нет"
                               : highestBidderName;

    currentBidLabel_->setText(
        QString("Текущая ставка: %1 $ | Лидер: %2")
            .arg(currentBid)
            .arg(bidder)
        );

    bidSpinBox_->setMinimum(currentBid + 1);

    if (bidSpinBox_->value() <= currentBid)
        bidSpinBox_->setValue(currentBid + 1);
}