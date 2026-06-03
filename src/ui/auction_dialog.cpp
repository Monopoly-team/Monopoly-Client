#include "ui/auction_dialog.hpp"

#include <QLabel>
#include <QIntValidator>
#include <QLineEdit>
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

    bidEdit_ = new QLineEdit(this);
    bidEdit_->setObjectName("auctionBidInput");
    bidEdit_->setPlaceholderText("Введите ставку");
    bidEdit_->setValidator(new QIntValidator(1, 999999, bidEdit_));

    bidButton_ = new QPushButton("Сделать ставку", this);
    bidButton_->setObjectName("primaryDialogButton");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(14);
    rootLayout->addWidget(titleLabel_);
    rootLayout->addWidget(timerLabel_);
    rootLayout->addWidget(currentBidLabel_);
    rootLayout->addWidget(bidEdit_);
    rootLayout->addWidget(bidButton_);

    connect(bidButton_, &QPushButton::clicked, this, [this]() {
        emit bidRequested(bidEdit_->text().toInt());
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

    bidEdit_->setPlaceholderText(QString("Минимум %1").arg(currentBid + 1));

    const int currentInput = bidEdit_->text().toInt();

    if (currentInput <= currentBid)
        bidEdit_->clear();
}