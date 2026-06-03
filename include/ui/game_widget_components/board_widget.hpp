#pragma once

#include "game/models/client_board_cell.hpp"
#include "network/client/tcp_client_controller.hpp"

#include <QWidget>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>
#include <QPainter>
#include <QRect>
#include <QTimer>
#include <QElapsedTimer>
#include <QHash>

enum class BoardCellSide
{
    Bottom,
    Left,
    Top,
    Right,
    Corner
};
class QContextMenuEvent;
class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget* parent = nullptr);
    ~BoardWidget() override;

    void addEvent(const QString& event);
    void clearEvents();
    void setCells(const QVector<ClientBoardCell>& cells);
    void setPlayers(const QVector<ClientGamePlayer>& players);
    void setLocalPlayerId(quint16 playerId);
signals:
    void messageSent(const QString& message);
    void buildBusinessRequested(int cellId);
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
private:
    void updateChatGeometry();
    void sendMessage();

    void drawCells(QPainter& painter);
    void drawCell(QPainter& painter, const QRect& rect, const ClientBoardCell* cell, BoardCellSide side);
    void drawCellByIndex(QPainter& painter, const QRect& rect, int index, BoardCellSide side);

    QVector<ClientBoardCell> createDefaultCells() const;

    QRect   cellRectByIndex(int index) const;
    QPoint  tokenPositionForCell(int cellIndex, int tokenIndex, int tokenCount) const;
    void    drawPlayerTokens(QPainter& painter);

    void            updateTokenAnimations();
    QPointF         currentTokenPosition(const ClientGamePlayer& player, int tokenIndex, int tokenCount) const;
    QVector<int>    buildMovePath(int from, int to) const;
    void            startTokenAnimation(quint16 playerId,int fromPosition,int toPosition);

    const ClientBoardCell* cellAtPoint(const QPoint& point) const;
    void showCellContextMenu(const QPoint& globalPosition, const ClientBoardCell& cell);
    QString cellInfoText(const ClientBoardCell& cell) const;
    void drawBuildingMarker(QPainter& painter, const QRect& rect, const ClientBoardCell& cell, BoardCellSide side);
    bool canBuildOnCellEvenly(const ClientBoardCell& cell) const;

    QString ownerNameById(quint16 playerId) const;


private:
    struct TokenAnimation
    {
        QVector<int> path;
        int currentStep = 0;

        QPointF from;
        QPointF to;

        float progress = 0.0f;
    };

    QFrame*                     eventChatArea_;
    QTextEdit*                  eventsView_;
    QLineEdit*                  chatInput_;
    QVector<ClientBoardCell>    cells_;
    QVector<ClientGamePlayer>   players_;

    QTimer* animationTimer_;
    QElapsedTimer animationClock_;

    QHash<quint16, QPointF> tokenPositions_;
    QHash<quint16, TokenAnimation> tokenAnimations_;
    QHash<quint16, int> playerPositions_;
    quint16 localPlayerId_ = 0;

};

