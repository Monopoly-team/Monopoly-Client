#pragma once
#include <QWidget>
#include <QVBoxLayout>

#include "board_widget.hpp"

class CenterGameWidget : public QWidget
{
    Q_OBJECT
signals:
    void messageSent(const QString& message);
public:
    explicit CenterGameWidget(QWidget* parent = nullptr);
    ~CenterGameWidget() override;

    void addEvent(const QString &event);
    void setPlayers(const QVector<ClientGamePlayer>& players);
    void setCells(const QVector<ClientBoardCell>& cells);
private:
    BoardWidget* boardWidget_;

    QVBoxLayout* rootLayout_;
};
