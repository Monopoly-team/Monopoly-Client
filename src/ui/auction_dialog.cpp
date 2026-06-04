#include "ui/auction_dialog.hpp"

#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QTimer>


AuctionDialog::AuctionDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("auctionDialog");
    setModal(false);
    setWindowTitle("Торги");
    setFixedSize(660, 340);
    setSizeGripEnabled(false);

    moveDebounceTimer_ = new QTimer(this);
    moveDebounceTimer_->setSingleShot(true);
    moveDebounceTimer_->setInterval(180);

    connect(moveDebounceTimer_, &QTimer::timeout, this, [this]() {
        isMovingByUser_ = false;
        flushPendingAuctionUpdate();
    });

    titleLabel_ = new QLabel(this);
    titleLabel_->setObjectName("auctionTitle");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(false);
    titleLabel_->setMinimumHeight(62);

    timerLabel_ = new QLabel(this);
    timerLabel_->setObjectName("auctionTimer");
    timerLabel_->setAlignment(Qt::AlignCenter);
    timerLabel_->setFixedHeight(44);

    currentBidLabel_ = new QLabel(this);
    currentBidLabel_->setObjectName("auctionBidLabel");
    currentBidLabel_->setAlignment(Qt::AlignCenter);
    currentBidLabel_->setMinimumHeight(30);

    bidEdit_ = new QLineEdit(this);
    bidEdit_->setObjectName("auctionBidInput");
    bidEdit_->setPlaceholderText("Введите ставку");
    bidValidator_ = new QIntValidator(1, 999999, bidEdit_);
    bidEdit_->setValidator(bidValidator_);

    bidButton_ = new QPushButton("Сделать ставку", this);
    bidButton_->setObjectName("primaryDialogButton");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setSizeConstraint(QLayout::SetNoConstraint);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(14);
    rootLayout->addWidget(titleLabel_);
    rootLayout->addWidget(timerLabel_);
    rootLayout->addWidget(currentBidLabel_);
    rootLayout->addWidget(bidEdit_);
    rootLayout->addWidget(bidButton_);


    connect(bidButton_, &QPushButton::clicked, this, [this]() {
        const int amount = bidEdit_->text().toInt();

        if (amount <= 0)
            return;

        emit bidRequested(amount);
        bidEdit_->clear();
    });
}

void AuctionDialog::updateAuction(
    int cellId,
    const QString& cellName,
    int secondsLeft,
    int currentBid,
    int minimumBid,
    const QString& highestBidderName
    )
{
    if (isMovingByUser_)
    {
        hasPendingAuctionUpdate_ = true;

        pendingCellId_ = cellId;
        pendingCellName_ = cellName;
        pendingSecondsLeft_ = secondsLeft;
        pendingCurrentBid_ = currentBid;
        pendingMinimumBid_ = minimumBid;
        pendingHighestBidderName_ = highestBidderName;

        return;
    }

    applyAuctionUpdate(
        cellId,
        cellName,
        secondsLeft,
        currentBid,
        minimumBid,
        highestBidderName
        );
}
void AuctionDialog::applyAuctionUpdate(
    int cellId,
    const QString& cellName,
    int secondsLeft,
    int currentBid,
    int minimumBid,
    const QString& highestBidderName
    )
{
    updateTitleIfNeeded(cellId, cellName);
    updateTimerIfNeeded(secondsLeft);
    updateBidInfoIfNeeded(currentBid, minimumBid, highestBidderName);
}
void AuctionDialog::moveEvent(QMoveEvent* event)
{
    isMovingByUser_ = true;
    moveDebounceTimer_->start();

    QDialog::moveEvent(event);
}
void AuctionDialog::flushPendingAuctionUpdate()
{
    if (!hasPendingAuctionUpdate_)
        return;

    hasPendingAuctionUpdate_ = false;

    applyAuctionUpdate(
        pendingCellId_,
        pendingCellName_,
        pendingSecondsLeft_,
        pendingCurrentBid_,
        pendingMinimumBid_,
        pendingHighestBidderName_
        );
}
void AuctionDialog::updateTitleIfNeeded(int cellId, const QString& cellName)
{
    if (cellId_ == cellId && cellName_ == cellName)
        return;

    cellId_ = cellId;
    cellName_ = cellName;

    titleLabel_->setText(QStringLiteral("Торги за «%1»").arg(cellName_));
}

void AuctionDialog::updateTimerIfNeeded(int secondsLeft)
{
    if (secondsLeft_ == secondsLeft)
        return;

    secondsLeft_ = secondsLeft;

    timerLabel_->setText(
        QStringLiteral("Осталось: %1 сек.").arg(secondsLeft_)
        );
}

void AuctionDialog::updateBidInfoIfNeeded(
    int currentBid,
    int minimumBid,
    const QString& highestBidderName
    )
{
    if (currentBid_ == currentBid &&
        minimumBid_ == minimumBid &&
        highestBidderName_ == highestBidderName)
    {
        return;
    }

    currentBid_ = currentBid;
    minimumBid_ = minimumBid;
    highestBidderName_ = highestBidderName;

    const QString bidder = highestBidderName_.isEmpty()
                               ? QStringLiteral("ставок пока нет")
                               : highestBidderName_;

    currentBidLabel_->setText(
        QStringLiteral("Текущая ставка: %1 $ | Лидер: %2")
            .arg(currentBid_)
            .arg(bidder)
        );

    bidEdit_->setPlaceholderText(
        QStringLiteral("Минимум %1").arg(minimumBid_)
        );

    if (bidValidator_)
        bidValidator_->setBottom(minimumBid_);
}