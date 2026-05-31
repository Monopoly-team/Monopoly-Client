#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <QVector>
#include <QJsonObject>
#include <QHash>
#include <QTimer>

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
public:
    TcpServerController(QObject* parent = nullptr);
    ~TcpServerController() override;

    bool startServer(quint16 port);

private:
    void handleMessage(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleConnectRequest(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleReadyChanged(QTcpSocket* senderSocket, const QJsonObject& message);
    void handleChatMessage(QTcpSocket* senderSocket, const QJsonObject &message);

    void sendToClient(QTcpSocket* client, const QJsonObject& message);
    void broadcastMessage(const QJsonObject& message);

    void broadcastLobbyUpdate();

    void checkGameStart();
    void startGame();
    void startCountdown();
    void cancelCountdown();
    bool areAllPlayersReady() const;
private:
    QTcpServer*                         server_;
    QVector<QTcpSocket*>                clients_;
    QHash<QTcpSocket*, ServerPlayer>    players_;
    quint16                             nextPlayerId_           = 1;
    QTcpSocket*                         admin_                  = nullptr;
    QTimer*                             startCountdownTimer_;
    int                                 countdownSecondsLeft_   = 5;
    bool                                countdownActive_        = false;

};

//TODO: Отправлять серверу список игроков