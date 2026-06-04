#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QMoveEvent;
class QTimer;
class QIntValidator;
class AuctionDialog : public QDialog
{
    Q_OBJECT
protected:
    void moveEvent(QMoveEvent* event) override;
signals:
    void bidRequested(int amount);

public:
    explicit AuctionDialog(QWidget* parent = nullptr);

    void updateAuction(
        int cellId,
        const QString& cellName,
        int secondsLeft,
        int currentBid,
        int minimumBid,
        const QString& highestBidderName
        );
private:
    void updateTitleIfNeeded(int cellId, const QString& cellName);
    void updateTimerIfNeeded(int secondsLeft);
    void updateBidInfoIfNeeded(int currentBid, int minimumBid, const QString& highestBidderName);
    void applyAuctionUpdate(int cellId, const QString& cellName, int secondsLeft, int currentBid, int minimumBid, const QString& highestBidderName);
    void flushPendingAuctionUpdate();
private:
    int     cellId_ = -1;
    QString cellName_;

    int     secondsLeft_ = -1;
    int     currentBid_ = -1;
    QString highestBidderName_;

    QLabel*      titleLabel_;
    QLabel*      timerLabel_;
    QLabel*      currentBidLabel_;
    QLineEdit*   bidEdit_;
    QPushButton* bidButton_;

    bool isMovingByUser_ = false;
    bool hasPendingAuctionUpdate_ = false;

    int pendingCellId_ = -1;
    QString pendingCellName_;
    int pendingSecondsLeft_ = -1;
    int pendingCurrentBid_ = -1;
    QString pendingHighestBidderName_;

    QTimer* moveDebounceTimer_;

    int minimumBid_ = 1;
    int pendingMinimumBid_ = 1;
    QIntValidator* bidValidator_ = nullptr;
};