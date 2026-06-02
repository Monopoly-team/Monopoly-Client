#pragma once

#include "center_card_widget.hpp"

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>

class MenuWidget : public CenterCardWidget
{
    Q_OBJECT
signals:
    void joinGameRequested();
    void createGameRequested();
public:
    explicit MenuWidget(QWidget* parent = nullptr);
    ~MenuWidget() override;
    QString serverIp() const;
    QString nickname() const;

    QPushButton *createButton() const;
    QPushButton *joinButton() const;

private:
    QLabel*         menuLabel_;

    QLineEdit*      nameEdit_;
    QLineEdit*      ipEdit_;

    QPushButton*    createButton_;
    QPushButton*    joinButton_;

    QHBoxLayout*    buttonsLayout_;
};


