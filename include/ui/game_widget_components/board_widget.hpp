#pragma once

#include "game/models/client_board_cell.hpp"

#include <QWidget>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>
#include <QPainter>
#include <QRect>

enum class BoardCellSide
{
    Bottom,
    Left,
    Top,
    Right,
    Corner
};

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget* parent = nullptr);
    ~BoardWidget() override;

    void addEvent(const QString& event);
    void clearEvents();
    void setCells(const QVector<ClientBoardCell>& cells);
signals:
    void messageSent(const QString& message);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateChatGeometry();
    void sendMessage();

    void drawCells(QPainter& painter);
    void drawCell(QPainter& painter, const QRect& rect, const ClientBoardCell* cell, BoardCellSide side);
    void drawCellByIndex(QPainter& painter, const QRect& rect, int index, BoardCellSide side);

    QVector<ClientBoardCell> createDefaultCells() const;

private:
    QFrame*                     eventChatArea_;
    QTextEdit*                  eventsView_;
    QLineEdit*                  chatInput_;
    QVector<ClientBoardCell>    cells_;
};

