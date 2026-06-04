#pragma once

#include <QObject>
#include <QHash>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>

#include "game/game_controller.hpp"

struct ServerPlayer
{
    QString nickname;
    quint16 id;
    bool ready = false;
    QString avatarPath;
};

class TcpServerController : public QObject
{
    Q_OBJECT

public:
    explicit TcpServerController(QObject* parent = nullptr);
    ~TcpServerController() override;

    bool startServer(quint16 port);
    bool isListening() const;

private slots:
    void onNewConnection();
    void onReadyRead();
    void onCountdownTick();
    void onDisconnect();
    void onAuctionTick();

private:
    struct AuctionState
    {
        bool active         = false;
        int secondsLeft     = 0;
        int cellId          = -1;
        int currentBid      = 0;
        int minimumBid      = 0;
        quint16 highestBidderId     = 0;
        QString highestBidderName;
        quint16 ownerTurnPlayerId   = 0;

        void reset()
        {
            active          = false;
            secondsLeft     = 0;
            cellId          = -1;
            currentBid      = 0;
            minimumBid      = 0;
            highestBidderId = 0;
            highestBidderName.clear();
            ownerTurnPlayerId = 0;
        }
    };

private:
    QString randomAvailableAvatarPath() const;

    void handleMessage(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleConnectRequest(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleReadyChanged(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleChatMessage(QTcpSocket* senderSocket, const QJsonObject& message);
    void handlePlayerAction(QTcpSocket* senderSocket, const QJsonObject& message);

    void handleAdminMessage(QTcpSocket* senderSocket, const QJsonObject& message);
    void handlePlayerMessage(QTcpSocket* senderSocket, const QJsonObject& message);

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

    void sendPurchaseOfferToCurrentPlayer(quint16 expectedPlayerId);
    void startAuction(quint16 requesterPlayerId);
    void clearPendingPurchaseOffer();
    void handleAuctionBid(quint16 playerId, const QJsonObject& payload);
    void finishAuction();
    void broadcastAuctionUpdate();

    void shutdownGame(const QString& reason);

    QJsonObject gameStartedMessage();

    void handleAdminAction(QTcpSocket* senderSocket, const QJsonObject& message);
    QTcpSocket* socketByPlayerId(quint16 playerId) const;

private:
    QTcpServer* server_ = nullptr;
    QVector<QTcpSocket*> clients_;
    QHash<QTcpSocket*, ServerPlayer> players_;
    quint16 nextPlayerId_ = 1;

    QTcpSocket* admin_ = nullptr;

    QTimer* startCountdownTimer_ = nullptr;
    int countdownSecondsLeft_ = 5;
    bool countdownActive_ = false;
    bool gameStarted_ = false;

    GameController* gameController_ = nullptr;

    QTimer* auctionTimer_ = nullptr;
    AuctionState auction_;

    quint16 purchaseOfferPendingPlayerId_ = 0;
    int purchaseOfferPendingCellId_ = -1;
};