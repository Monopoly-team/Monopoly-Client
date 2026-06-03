#pragma once

#include <QDialog>

class QLabel;
class QPushButton;

class PurchaseOfferDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PurchaseOfferDialog(QWidget* parent = nullptr);

    void setOffer(int cellId, const QString& cellName, int price, int playerBalance);

signals:
    void buyRequested();
    void auctionRequested();

private:
    int cellId_ = -1;

    QLabel*         titleLabel_;
    QLabel*         priceLabel_;
    QPushButton*    buyButton_;
    QPushButton*    auctionButton_;
};