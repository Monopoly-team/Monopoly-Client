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
    server_                 = new QTcpServer(this);
    startCountdownTimer_    = new QTimer(this);
    gameController_         = new GameController(this);

    connect(startCountdownTimer_, &QTimer::timeout, this, &TcpServerController::onCountdownTick);
    connect(server_, &QTcpServer::newConnection, this, &TcpServerController::onNewConnection);
}

TcpServerController::~TcpServerController() = default;

bool TcpServerController::startServer(quint16 port)
{
    if (server_->isListening())
    {
        qDebug() << "[Server] already listening";
        return false;
    }
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
    //TODO: не принимать если игра началась / максимум игроков / есть админ
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
    else if (players_.contains(clientSocket))
    {
        const quint16 playerId = players_[clientSocket].id;

        players_.remove(clientSocket);
        gameController_->removePlayer(playerId);

        if (players_.isEmpty())
        {
            qDebug() << "[Server] No players left. Closing game server.";

            gameStarted_ = false;
            countdownActive_ = false;
            startCountdownTimer_->stop();

            if (server_->isListening())
                server_->close();

            if (admin_)
            {
                admin_->disconnectFromHost();
                admin_ = nullptr;
            }
        }
        else
        {
            broadcastLobbyUpdate();
            if (gameStarted_)
                broadcastGameUpdate();
        }
    }

    clientSocket->deleteLater();
    sendLobbyPlayersToAdmin();
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

void TcpServerController::onCountdownTick()
{
    if(!areAllPlayersReady())
    {
        cancelCountdown();
        return;
    }
    --countdownSecondsLeft_;

    if(countdownSecondsLeft_ <= 0)
    {
        countdownActive_ = false;
        startCountdownTimer_->stop();

        startGame();
        return;
    }
    QJsonObject payload;
    payload["secondsLeft"] = countdownSecondsLeft_;
    broadcastToPlayers(
        NetworkMessage::create(
            "countdown_update",
            SERVER_ID,
            payload
            )
        );
}

void TcpServerController::handleMessage(QTcpSocket* senderSocket, const QJsonObject& message)
{
    const QString type = message["type"].toString();

    qDebug() << "[Server] received" << type << message;

    if (type == "connect_request")
    {
        handleConnectRequest(senderSocket, message);
        return;
    }

    if (senderSocket == admin_)
    {
        handleAdminMessage(senderSocket, message);
        return;
    }

    if (players_.contains(senderSocket))
    {
        handlePlayerMessage(senderSocket, message);
        return;
    }

    qDebug() << "[Server] message from unknown socket:" << type;
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


        admin_ = senderSocket;
        sendToAdmin(NetworkMessage::create(
                         "connect_accept",
                         SERVER_ID,
                         acceptedPayload
                         )
                     );
        qDebug() << "[Server] Admin connected successfully";

        sendLobbyPlayersToAdmin();
        if(gameStarted_) sendToAdmin(gameStartedMessage());

        return;
    }
    if (gameStarted_)
    {
        QJsonObject payload;
        payload["code"] = "game_already_started";
        payload["message"] = "Игра уже началась";

        sendToClient(
            senderSocket,
            NetworkMessage::create("error", SERVER_ID, payload)
            );

        senderSocket->disconnectFromHost();
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
    gameController_->addPlayer(player.id, player.nickname);

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

    gameController_->takeEvents();

    broadcastLobbyUpdate();
    sendLobbyPlayersToAdmin();

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
    broadcastToPlayers(message);
}

void TcpServerController::sendLobbyPlayersToAdmin()
{
    if (!admin_ || admin_->state() != QAbstractSocket::ConnectedState)
        return;

    QJsonArray playersArray;

    for (const ServerPlayer& player : players_)
    {
        QJsonObject obj;
        obj["id"] = player.id;
        obj["name"] = player.nickname;
        obj["isConnected"] = true;

        playersArray.append(obj);
    }

    QJsonObject payload;
    payload["players"] = playersArray;

    sendToAdmin(
        NetworkMessage::create(
            "players_list_lobby",
            SERVER_ID,
            payload
            )
        );
}

void TcpServerController::checkGameStart()
{
    if (areAllPlayersReady())
    {
        startCountdown();
    }
    else
    {
        cancelCountdown();
    }
}
QJsonObject TcpServerController::gameStartedMessage()
{
    return NetworkMessage::create(
        "game_started",
        SERVER_ID,
        {}
        );
}

void TcpServerController::startGame()
{
    if (gameStarted_)
        return;

    if (!gameController_->startGame())
    {
        broadcastGameEvents();
        return;
    }

    gameStarted_ = true;

    qDebug() << "[Server] Starting game";

    broadcastMessage(gameStartedMessage());
    broadcastGameUpdate();
}

void TcpServerController::broadcastGameState()
{
    broadcastMessage(
        NetworkMessage::create(
            "game_state",
            SERVER_ID,
            gameController_->gameStateToJson()
            )
        );
}

void TcpServerController::broadcastGameEvents()
{
    const QStringList events = gameController_->takeEvents();

    for (const QString& eventText : events)
    {
        QJsonObject payload;
        payload["text"] = eventText;

        broadcastMessage(
            NetworkMessage::create(
                "game_event",
                SERVER_ID,
                payload
                )
            );
    }
}

void TcpServerController::broadcastGameUpdate()
{
    broadcastGameState();
    broadcastGameEvents();
}

void TcpServerController::startCountdown()
{
    if(countdownActive_)
        return;
    countdownActive_ = true;
    countdownSecondsLeft_ = 5;

    QJsonObject payload;
    payload["secondsLeft"] = countdownSecondsLeft_;
    broadcastToPlayers(
        NetworkMessage::create(
                "countdown_update",
                SERVER_ID,
                payload
            )
        );
    startCountdownTimer_->start(1000);
}

void TcpServerController::cancelCountdown()
{
    if (!countdownActive_)
        return;
    countdownActive_ = false;
    startCountdownTimer_->stop();

    broadcastToPlayers(
        NetworkMessage::create(
            "countdown_cancelled",
            SERVER_ID,
            {}
            )
        );
}

bool TcpServerController::areAllPlayersReady() const
{
    if(players_.size() < 2)
        return false;
    for(const ServerPlayer& player : players_)
    {
        if(!player.ready)
            return false;
    }
    return true;
}

bool TcpServerController::isListening() const
{
    return server_->isListening();
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

void TcpServerController::broadcastToPlayers(const QJsonObject& message)
{
    for (auto it = players_.begin(); it != players_.end(); ++it)
    {
        QTcpSocket* socket = it.key();
        sendToClient(socket, message);
    }

    qDebug() << "[Server] broadcasted to players"
             << message["type"].toString();
}

void TcpServerController::sendToAdmin(const QJsonObject& message)
{
    if (!admin_)
        return;

    sendToClient(admin_, message);

    qDebug() << "[Server] sent to admin"
             << message["type"].toString();
}

void TcpServerController::handleChatMessage(QTcpSocket* senderSocket, const QJsonObject& message)
{
    if (!players_.contains(senderSocket))
        return;

    QJsonObject payload = message["payload"].toObject();

    payload["playerId"] = players_[senderSocket].id;
    payload["nickname"] = players_[senderSocket].nickname;

    QJsonObject broadcast =
        NetworkMessage::create(
            "chat_message",
            SERVER_ID,
            payload
            );

    broadcastToPlayers(broadcast);
}

void TcpServerController::handleAdminMessage(QTcpSocket* senderSocket, const QJsonObject& message)
{
    Q_UNUSED(senderSocket);

    const QString type = message["type"].toString();

    if (type == "admin_action")
    {
        handleAdminAction(message);
        return;
    }

    qDebug() << "[Server] unhandled admin type:" << type;
}

void TcpServerController::handleAdminAction(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();
    const QString action = payload["action"].toString();

    if(action == "kick")
    {
        handleKickPlayer(payload["playerId"].toInt());
        return;
    }

    qDebug() << "[Server] unknown admin action:" << action;
}

void TcpServerController::handleKickPlayer(quint16 playerId)
{
    for(auto it = players_.begin(); it != players_.end(); ++it)
    {
        if(it.value().id != playerId)
            continue;
        if (it.value().id == 1)
        {
            qDebug() << "[Server] Host kicked. Shutting down game.";
            shutdownGame("host_kicked");
            return;
        }
        QTcpSocket* socket = it.key();

        qDebug() << "[Server] kicking player"
                 << it.value().nickname
                 << playerId;

        QJsonObject payload;
        payload["reason"] = "kicked_by_admin";

        sendToClient(
            socket,
            NetworkMessage::create(
                "server_disconnect",
                SERVER_ID,
                payload
                )
            );

        socket->disconnectFromHost();

        return;
    }

    qDebug() << "[Server] player not found:" << playerId;
}

void TcpServerController::handlePlayerMessage(QTcpSocket* senderSocket, const QJsonObject& message)
{
    const QString type = message["type"].toString();

    if (type == "chat_message")
    {
        handleChatMessage(senderSocket, message);
        return;
    }

    if (type == "ready_changed")
    {
        handleReadyChanged(senderSocket, message);
        return;
    }
    if (type == "player_action")
    {
        handlePlayerAction(senderSocket, message);
        return;
    }

    qDebug() << "[Server] unhandled player type:" << type;
}

void TcpServerController::handlePlayerAction(QTcpSocket* senderSocket, const QJsonObject& message)
{
    if (!players_.contains(senderSocket))
    {
        qDebug() << "[Server] player_action from unknown socket";
        return;
    }

    if (!gameStarted_)
    {
        QJsonObject payload;
        payload["text"] = "Действие отклонено: игра ещё не началась.";

        sendToClient(
            senderSocket,
            NetworkMessage::create(
                "game_event",
                SERVER_ID,
                payload
                )
            );

        return;
    }

    const ServerPlayer& player = players_[senderSocket];
    const QJsonObject payload = message["payload"].toObject();

    qDebug() << "[Server] player action from"
             << player.nickname
             << player.id
             << payload;

    gameController_->handlePlayerAction(player.id, payload);

    broadcastGameUpdate();
}


void TcpServerController::shutdownGame(const QString& reason)
{
    QJsonObject payload;
    payload["reason"] = reason;

    QJsonObject message = NetworkMessage::create(
        "server_disconnect",
        SERVER_ID,
        payload
        );
        if (admin_)
        {
            sendToClient(admin_, message);
            admin_->disconnectFromHost();
            admin_ = nullptr;
        }
    for (auto it = players_.begin(); it != players_.end(); ++it)
    {
        sendToClient(it.key(), message);
        it.key()->disconnectFromHost();
    }



    players_.clear();
    clients_.clear();

    gameStarted_ = false;
    countdownActive_ = false;
    startCountdownTimer_->stop();
    nextPlayerId_ = 1;
    if (server_->isListening())
        server_->close();

    qDebug() << "[Server] Game shutdown:" << reason;
}