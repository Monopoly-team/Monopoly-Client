#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <QVector>
#include <QJsonObject>
#include <QHash>
#include <QTimer>

#include "game/game_controller.hpp"

struct ServerPlayer
{
    QString     nickname;
    quint16     id;
    bool        ready = false;
};

class TcpServerController : public QObject
{

private slots:
    void onNewConnection();
    void onReadyRead();
    void onCountdownTick();
    void onDisconnect();
    void onAuctionTick();
public:
    TcpServerController(QObject* parent = nullptr);
    ~TcpServerController() override;

    bool startServer(quint16 port);
    bool isListening() const;
private:
    void handleMessage(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleConnectRequest(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleReadyChanged(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleChatMessage(QTcpSocket* senderSocket, const QJsonObject &message);
    void handlePlayerAction(QTcpSocket* senderSocket, const QJsonObject& message);

    void handleAdminMessage(QTcpSocket* senderSocket, const QJsonObject &message);
    void handlePlayerMessage(QTcpSocket* senderSocket, const QJsonObject &message);

    void handleAdminAction(const QJsonObject& message);
    void handleKickPlayer(quint16 playerId);


    void sendToClient(QTcpSocket* client, const QJsonObject& message);
    void broadcastMessage(const QJsonObject& message);
    void broadcastToPlayers(const QJsonObject& message);
    void sendToAdmin(const QJsonObject& message);

    void broadcastLobbyUpdate();
    void sendLobbyPlayersToAdmin();

    void broadcastGameState();
    void broadcastGameEvents();
    void broadcastGameUpdate();

    void checkGameStart();
    void startGame();
    void startCountdown();
    void cancelCountdown();
    bool areAllPlayersReady() const;

    void sendPurchaseOfferToCurrentPlayer();
    void startAuction();
    void handleAuctionBid(quint16 playerId, const QJsonObject& payload);
    void finishAuction();
    void broadcastAuctionUpdate();

    void shutdownGame(const QString& reason);

    QJsonObject gameStartedMessage();

private:
    QTcpServer*                         server_;
    QVector<QTcpSocket*>                clients_;
    QHash<QTcpSocket*, ServerPlayer>    players_;
    quint16                             nextPlayerId_           = 1;
    QTcpSocket*                         admin_                  = nullptr;
    QTimer*                             startCountdownTimer_;
    int                                 countdownSecondsLeft_   = 5;
    bool                                countdownActive_        = false;
    bool                                gameStarted_            = false;
    GameController*                     gameController_;

    QTimer*                             auctionTimer_ = nullptr;

    bool                                auctionActive_ = false;
    int                                 auctionSecondsLeft_ = 0;
    int                                 auctionCellId_ = -1;
    int                                 auctionCurrentBid_ = 0;
    quint16                             auctionHighestBidderId_ = 0;
    QString                             auctionHighestBidderName_;
    quint16                             auctionOwnerTurnPlayerId_ = 0;


};

//TODO: Отправлять серверу список игроков