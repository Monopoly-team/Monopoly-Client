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
#include <QPixmap>

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
    void addChatMessage(quint16 playerId, const QString& nickname, const QString& text);
    void setWinnerId(quint16 winnerId);
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

    const ClientGamePlayer* playerById(quint16 playerId) const;
    const ClientGamePlayer* playerAtEventStart(const QString& eventText, QString* matchedNickname = nullptr) const;
    const ClientGamePlayer* playerInTurnEvent(const QString& eventText, QString* matchedNickname = nullptr) const;

    QColor displayColorForPlayer(const ClientGamePlayer& player) const;
    QString iconForEventText(const QString& text) const;

    QColor ownerColorForCell(const ClientBoardCell& cell) const;
    QColor backgroundColorForCell(const ClientBoardCell* cell) const;

    void appendChatLine(const ClientGamePlayer& player, const QString& text);
    void appendPlayerEventLine(const ClientGamePlayer& player, const QString& text);
    void appendTurnEventLine(const QString& eventText, const ClientGamePlayer& player, const QString& nickname);
    void appendSystemLine(const QString& text, bool withEventIcon = true);
    QString pixmapCacheKey(const QString& path, const QSize& targetSize) const;
    QPixmap cachedCellPixmap(const QString& path, const QSize& targetSize);

private:
    struct TokenAnimation
    {
        QVector<int> path;
        int currentStep = 0;

        QPointF from;
        QPointF to;

        float progress = 0.0f;
    };

    QFrame*                         eventChatArea_;
    QTextEdit*                      eventsView_;
    QLineEdit*                      chatInput_;
    QVector<ClientBoardCell>        cells_;
    QVector<ClientGamePlayer>       players_;

    QTimer*                         animationTimer_;
    QElapsedTimer                   animationClock_;

    QHash<quint16, QPointF>         tokenPositions_;
    QHash<quint16, TokenAnimation>  tokenAnimations_;
    QHash<quint16, int>             playerPositions_;
    QHash<QString, QPixmap>         pixmapCache_;
    quint16                         localPlayerId_ = 0;
    quint16                         winnerId_ = 0;

};

