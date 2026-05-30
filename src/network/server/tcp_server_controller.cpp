#include "network/server/tcp_server_controller.hpp"
#include "network/network_message.hpp"
#include "network_constants.hpp"

#include <QTimer>
#include <QHostAddress>
#include <QDebug>
#include <QJsonArray>


TcpServerController::TcpServerController(QObject *parent)
    : QObject(parent)
{
    server_ = new QTcpServer(this);

    connect(server_, &QTcpServer::newConnection, this, &TcpServerController::onNewConnection);
}

TcpServerController::~TcpServerController() = default;

bool TcpServerController::startServer(quint16 port)
{
    if(!server_->listen(QHostAddress::Any,port))
    {
        qDebug() << "[Server] start failed" << server_->errorString();
        return false;
    }
    qDebug() << "[Server] started on port " << port;
    return true;
}

void TcpServerController::onNewConnection()
{
    QTcpSocket* clientSocket = server_->nextPendingConnection();
    if(!clientSocket)
        return;

    clients_.push_back(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead,this, &TcpServerController::onReadyRead);

    qDebug() << "[Server] Client accepted:"
             << QHostAddress(clientSocket->peerAddress().toIPv4Address()).toString()
             << clientSocket->peerPort();

    connect(clientSocket, &QTcpSocket::disconnected,this, &TcpServerController::onDisconnect);
}

void TcpServerController::onDisconnect()
{
    auto* clientSocket = qobject_cast<QTcpSocket*>(sender());

    qDebug() << "[Server] Client disconnected"
             << QHostAddress(clientSocket->peerAddress().toIPv4Address()).toString()
             << clientSocket->peerPort();

    clients_.removeOne(clientSocket);

    if(admin_==clientSocket)
    {
        qDebug() << "[Server] Admin Disconnected";
        admin_ = nullptr;
    }
    else if(players_.contains(clientSocket))
    {
        players_.remove(clientSocket);
        broadcastLobbyUpdate();
    }

    clientSocket->deleteLater();
}

void TcpServerController::onReadyRead()
{
    auto* clientSocket = qobject_cast<QTcpSocket*>(sender());

    if(!clientSocket)
        return;

    while(clientSocket->canReadLine())
    {
        const QByteArray line = clientSocket->readLine().trimmed();
        const QJsonObject message = NetworkMessage::deserialize(line);

        if(!NetworkMessage::isValid(message))
        {
            qDebug() << "[Server] Invalid message from client: " << line;
            continue;
        }

        handleMessage(clientSocket, message);
    }
}



void TcpServerController::handleMessage(QTcpSocket *senderSocket, const QJsonObject &message)
{
    const QString type = message["type"].toString();

    qDebug() << "[Server] received " << type << message;

    if(type == "connect_request")
    {
        handleConnectRequest(senderSocket,message);
        return;
    }
    if(type == "chat_message")
    {
        broadcastMessage(message);
        return;
    }
    if(type == "ready_changed")
    {
        handleReadyChanged(senderSocket,message);
        return;
    }
    qDebug() << "[Server] unhandled type: " << type;
}

void TcpServerController::handleConnectRequest(QTcpSocket *senderSocket, const QJsonObject &message)
{

    const QJsonObject   payload  = message["payload"].toObject();
    QString             nickname = payload["nickname"].toString().trimmed();
    const quint16       senderId = static_cast<quint16>(message["senderId"].toInt());

    if(players_.contains(senderSocket))
    {
        qDebug() << "[Server] socket already exist as player";
        return;
    }

    if(senderId == ADMIN_ID)
    {
        if(admin_ != nullptr)
        {
            QJsonObject deniedPayload;
            deniedPayload["code"]   = "admin_already_connected";
            deniedPayload["message"] = "admin already exists";
            sendToClient(senderSocket,
                         NetworkMessage::create(
                             "error",
                             SERVER_ID,
                             deniedPayload
                             )
                         );
            senderSocket->disconnectFromHost();
            qDebug() << "[Server] admin limit reached. Disconnected socket";
            return;
        }

        QJsonObject acceptedPayload;
        acceptedPayload["access"] = "accepted";
        sendToClient(senderSocket,
                     NetworkMessage::create(
                        "connect_accept",
                        SERVER_ID,
                        acceptedPayload
                         )
                     );
        admin_ = senderSocket;
        qDebug() << "[Server] Admin connected successfully";

        return;
    }

    if (players_.size() >= MAX_PLAYERS)
    {
        QJsonObject payload;
        payload["code"] = "lobby_full";
        payload["message"] = "Лобби заполнено";

        sendToClient(senderSocket, NetworkMessage::create("error", 0, payload));
        senderSocket->disconnectFromHost();
        return;
    }



    if(nickname.isEmpty())
    {
        nickname = "Игрок";
    }
    ServerPlayer player;

    player.id       = nextPlayerId_++;
    player.nickname = nickname;
    player.ready    = false;

    players_.insert(senderSocket,player);

    QJsonObject acceptedPayload;

    acceptedPayload["playerId"] = player.id;
    acceptedPayload["nickname"] = player.nickname;

    QJsonObject acceptedMessage =
        NetworkMessage::create(
            "connect_accepted",
            SERVER_ID,
            acceptedPayload
        );

    sendToClient(senderSocket, acceptedMessage);
    broadcastLobbyUpdate();

}

void TcpServerController::handleReadyChanged(QTcpSocket *senderSocket, const QJsonObject &message)
{
    if(!players_.contains(senderSocket))
    {
        qDebug() << "[Server] ready_changed from unknown socket";
        return;
    }

    const QJsonObject payload = message["payload"].toObject();
    const bool ready = payload["ready"].toBool();

    players_[senderSocket].ready = ready;

    qDebug() << "[Server] player"
             << players_[senderSocket].nickname
             << "changed ready status to "
             << ready;
    broadcastLobbyUpdate();
    checkGameStart();
}

void TcpServerController::broadcastLobbyUpdate()
{
    // TODO: шлёт lobby update всем clients_, даже тем, кто ещё не отправил connect_request.
    // рассылать только авторизованным игрокам.

    QJsonArray playersArray;
    for(const ServerPlayer& player : players_)
    {
        QJsonObject playerObject;

        playerObject["id"]       = player.id;
        playerObject["nickname"] = player.nickname;
        playerObject["ready"]    = player.ready;

        playersArray.append(playerObject);
    }

    QJsonObject payload;

    payload["players"] = playersArray;
    QJsonObject message =
        NetworkMessage::create(
            "lobby_update",
            SERVER_ID,
            payload
        );
    broadcastMessage(message);
}

void TcpServerController::checkGameStart()
{
    if(players_.size() < 2 )
        return;
    for(const ServerPlayer& player : players_)
    {
        if(!player.ready)
            return;
    }
    startGame();
}

void TcpServerController::startGame()
{
    qDebug() << "[Server] Starting game";
    QJsonObject payload;
    payload["playerCount"] = players_.size();
    broadcastMessage(
        NetworkMessage::create(
                "game_started",
                SERVER_ID,
                payload
            )
        );
}


void TcpServerController::sendToClient(QTcpSocket *client, const QJsonObject &message)
{
    if(!client || client->state() != QAbstractSocket::ConnectedState)
        return;
    client->write(NetworkMessage::serialize(message));
    client->flush();
}

void TcpServerController::broadcastMessage(const QJsonObject &message)
{
    for(QTcpSocket* client : clients_)
        sendToClient(client, message);

    qDebug() << "[Server] broadcasted " << message["type"].toString();
}