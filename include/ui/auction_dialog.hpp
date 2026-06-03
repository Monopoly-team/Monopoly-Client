#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

class AuctionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuctionDialog(QWidget* parent = nullptr);

    void updateAuction(
        int cellId,
        const QString& cellName,
        int secondsLeft,
        int currentBid,
        const QString& highestBidderName
        );

signals:
    void bidRequested(int amount);

private:
    int cellId_ = -1;

    QLabel* titleLabel_;
    QLabel* timerLabel_;
    QLabel* currentBidLabel_;
    QLineEdit*   bidEdit_;
    QPushButton* bidButton_;
};