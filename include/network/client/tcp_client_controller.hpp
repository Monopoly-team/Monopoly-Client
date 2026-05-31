#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QVector>

struct ClientLobbyPlayer
{
    QString     nickname;
    quint16     id    = 0;
    bool        ready = false;
};

struct ClientGamePlayer
{
    quint16 id = 0;
    QString nickname;
    qint32 balance = 0;
    quint8 position = 0;
    QString color;
};

class TcpClientController : public QObject
{
    Q_OBJECT
signals:
    void connectedToServer();
    void disconnectedFromServer();
    void messageReceived(const QJsonObject& message);
    void errorOccurred(const QString& error);
    void lobbyUpdated(const QVector<ClientLobbyPlayer>& players);
    void gameStarted();
    void countdownUpdated(int secondsLeft);
    void countdownCancelled();
    void chatMessageReceived(const QString& nickname, const QString& text);
    void gameEventReceived(const QString& text);
    void gamePlayersUpdated(const QVector<ClientGamePlayer>& players);
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError error);

public:
    TcpClientController(QObject* parent = nullptr);
    ~TcpClientController() override;

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    void sendMessage(const QJsonObject& message);
    quint16 playerId() const;
private:
    void handleMessage(const QJsonObject& message);
    void handleConnectAccepted(const QJsonObject& message);
    void handleLobbyUpdate(const QJsonObject& message);
    void handleGameStarted(const QJsonObject& message);

    void handleChatMessage(const QJsonObject& message);
    void handleGameEvent(const QJsonObject& message);
    void handleGameState(const QJsonObject& message);
    void handleError(const QJsonObject& message);
private:
    QTcpSocket* socket_;
    quint16     playerId_ = 0;
};
// TODO: Добавить обработку ready_changed и lobby_update на клиенте.