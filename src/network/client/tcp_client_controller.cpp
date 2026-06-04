#include "network/client/tcp_client_controller.hpp"
#include "network/network_message.hpp"

#include <QDebug>
#include <QJsonArray>


namespace {

CellType cellTypeFromString(const QString& value)
{
    const QString type = value.trimmed().toLower();

    if (type == "corner")
        return CellType::Corner;

    if (type == "extra_business")
        return CellType::ExtraBusiness;

    if (type == "chance")
        return CellType::Chance;

    if (type == "community_chest")
        return CellType::CommunityChest;

    return CellType::Business;
}

BusinessGroup businessGroupFromString(const QString& value)
{
    QString group = value.trimmed().toLower();
    group.replace("_", "");

    if (group == "pharmacy")
        return BusinessGroup::Pharmacy;

    if (group == "clothes")
        return BusinessGroup::Clothes;

    if (group == "foodmarket")
        return BusinessGroup::FoodMarket;

    if (group == "darkstore")
        return BusinessGroup::DarkStore;

    if (group == "marketplace")
        return BusinessGroup::Marketplace;

    if (group == "bank")
        return BusinessGroup::Bank;

    if (group == "gamestudio")
        return BusinessGroup::GameStudio;

    if (group == "it")
        return BusinessGroup::IT;

    if (group == "iteducation")
        return BusinessGroup::ITEducation;

    return BusinessGroup::None;
}

}


TcpClientController::TcpClientController(QObject *parent)
    : QObject(parent)
{
    socket_ = new QTcpSocket(this);

    connect(socket_, &QTcpSocket::connected,    this, &TcpClientController::onConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &TcpClientController::onDisconnected);
    connect(socket_, &QTcpSocket::readyRead,    this, &TcpClientController::onReadyRead);
    connect(socket_, &QTcpSocket::errorOccurred,this, &TcpClientController::onErrorOccurred);
}

TcpClientController::~TcpClientController() = default;

void TcpClientController::connectToServer(const QString& host, quint16 port)
{
    qDebug() << "[Client] Connecting to server" << host << port;
    socket_->connectToHost(host,port);
}

void TcpClientController::disconnectFromServer()
{
    qDebug() << "[Client] Disconnecting from server";
    socket_->disconnectFromHost();
}

void TcpClientController::sendMessage(const QJsonObject &message)
{
    if(socket_->state() != QAbstractSocket::ConnectedState)
    {
        qDebug() << "[Client] Failed to sendMessage"
                 << " socket state is" << socket_->state();
        return;
    }
    const QByteArray data = NetworkMessage::serialize(message);
    socket_->write(data);
    socket_->flush();

    qDebug() << "[Client] Sent:" << data;
}

quint16 TcpClientController::playerId() const
{
    return playerId_;
}

void TcpClientController::handleMessage(const QJsonObject& message)
{
    const QString& type = message["type"].toString();

    if(type == "connect_accepted")
    {
        handleConnectAccepted(message);
        return;
    }
    if(type == "lobby_update")
    {
        handleLobbyUpdate(message);
        return;
    }
    if(type == "chat_message")
    {
        handleChatMessage(message);
        return;
    }
    if(type == "game_started")
    {
        handleGameStarted(message);
        return;
    }
    if(type == "countdown_update")
    {
        const QJsonObject payload = message["payload"].toObject();
        emit countdownUpdated(payload["secondsLeft"].toInt());
        return;
    }
    if(type == "countdown_cancelled")
    {
        emit countdownCancelled();
        return;
    }
    if (type == "game_event")
    {
        handleGameEvent(message);
        return;
    }
    if (type == "game_state")
    {
        handleGameState(message);
        return;
    }
    if(type == "server_disconnect")
    {
        handleServerDisconnect(message);
        return;
    }
    if (type == "error")
    {
        handleError(message);
        return;
    }
    if (type == "purchase_offer")
    {
        handlePurchaseOffer(message);
        return;
    }

    if (type == "auction_update")
    {
        handleAuctionUpdate(message);
        return;
    }

    if (type == "auction_finished")
    {
        handleAuctionFinished(message);
        return;
    }
    qDebug() << "[Client] Unhandled type: " << type;
}
void TcpClientController::handleServerDisconnect(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();

    emit serverDisconnectRequested(
        payload["reason"].toString()
        );

    socket_->disconnectFromHost();
}
void TcpClientController::handleConnectAccepted(const QJsonObject &message)
{
    const QJsonObject payload = message["payload"].toObject();

    playerId_ = static_cast<quint16>(payload["playerId"].toInt());

    qDebug() << "[Client] assigned playerId: " << playerId_;
}

void TcpClientController::handleLobbyUpdate(const QJsonObject &message)
{
    qDebug() << "[Client] lobby update received";
    qDebug() << "[Client] lobby update raw:" << message;
    QVector<ClientLobbyPlayer> players;

    const QJsonObject payload = message["payload"].toObject();
    const QJsonArray  playersArray = payload["players"].toArray();

    for(const QJsonValue& value : playersArray)
    {
        const QJsonObject object = value.toObject();

        ClientLobbyPlayer player;

        player.id       = static_cast<quint16>(object["id"].toInt());
        player.nickname = object["nickname"].toString();
        player.avatarPath = object["avatarPath"].toString();
        player.ready    = object["ready"].toBool();

        players.push_back(player);
    }
    qDebug() << "[Client] players parsed:" << players.size();
    emit lobbyUpdated(players);
}

void TcpClientController::handleChatMessage(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();

    const quint16 playerId = static_cast<quint16>(payload["playerId"].toInt());
    const QString nickname = payload["nickname"].toString();
    const QString text = payload["text"].toString();

    emit chatMessageReceived(playerId, nickname, text);
}

void TcpClientController::handleGameEvent(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();
    const QString text = payload["text"].toString();

    emit gameEventReceived(text);
}

void TcpClientController::handleGameState(const QJsonObject& message)
{

    ClientGameState state;

    const QJsonObject payload = message["payload"].toObject();

    state.status = payload["status"].toString();
    state.currentPlayerId = static_cast<quint16>(payload["currentPlayerId"].toInt());
    state.lastDiceValue = payload["lastDiceValue"].toInt();
    state.lastDiceFirst = payload["lastDiceFirst"].toInt(1);
    state.lastDiceSecond = payload["lastDiceSecond"].toInt(1);
    state.isGameOver = payload["isGameOver"].toBool();
    state.winnerId = static_cast<quint16>(payload["winnerId"].toInt());

    const QJsonArray playersArray = payload["players"].toArray();

    for (const QJsonValue& value : playersArray)
    {
        const QJsonObject object = value.toObject();

        ClientGamePlayer player;

        player.id = static_cast<quint16>(object["id"].toInt());
        player.nickname = object["nickname"].toString();
        player.avatarPath = object["avatarPath"].toString();
        player.balance = object["balance"].toInt();
        player.position = static_cast<quint8>(object["position"].toInt());
        player.color = object["color"].toString();

        player.isBankrupt = object["isBankrupt"].toBool();
        player.active = object["active"].toBool(true);
        player.isInJail = object["isInJail"].toBool();
        player.isCurrentTurn = player.id == state.currentPlayerId;
        player.ownedPropertiesCount = object["ownedProperties"].toArray().size();

        state.players.push_back(player);
    }

    const QJsonArray cellsArray = payload["cells"].toArray();

    for (const QJsonValue& value : cellsArray)
    {
        const QJsonObject object = value.toObject();

        ClientBoardCell cell;

        cell.id = static_cast<quint8>(object["id"].toInt());
        cell.name               = object["name"].toString();
        cell.type               = cellTypeFromString(object["type"].toString());
        cell.group              = businessGroupFromString(object["group"].toString());
        cell.price              = object["price"].toInt();
        cell.rent               = object["rent"].toInt();
        cell.buildingCost       = object["buildingCost"].toInt();
        cell.buildingLevel      = object["buildingLevel"].toInt();
        cell.maxBuildingLevel   = object["maxBuildingLevel"].toInt();
        cell.ownerId            = static_cast<quint16>(object["ownerId"].toInt());

        state.cells.push_back(cell);
    }

    qDebug() << "[Client] game_state parsed:"
             << "players =" << state.players.size()
             << "cells =" << state.cells.size()
             << "currentPlayerId =" << state.currentPlayerId
             << "lastDiceValue =" << state.lastDiceValue;

    emit gamePlayersUpdated(state.players);
    emit gameStateUpdated(state);
}

void TcpClientController::handleError(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();

    const QString code = payload["code"].toString();
    const QString text = payload["message"].toString();

    qDebug() << "[Client] server error:" << code << text;

    emit errorOccurred(text.isEmpty() ? code : text);
}

void TcpClientController::handleGameStarted(const QJsonObject &message)
{
    Q_UNUSED(message);

    qDebug() << "[Client] Game started";

    emit gameStarted();
}



void TcpClientController::onConnected()
{
    qDebug() << "[Client] Connected to server";
    emit connectedToServer();
}

void TcpClientController::onDisconnected()
{
    qDebug() << "[Client] Disconnected from server";
    emit disconnectedFromServer();
}

void TcpClientController::onReadyRead()
{
    while(socket_->canReadLine())
    {
        const QByteArray line = socket_->readLine().trimmed();

        const QJsonObject message =
            NetworkMessage::deserialize(line);

        if(!NetworkMessage::isValid(message))
            continue;

        handleMessage(message);
    }
}

void TcpClientController::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    qDebug() << "[Client] socket error: " << socket_->errorString();

    emit errorOccurred(socket_->errorString());
}

void TcpClientController::handlePurchaseOffer(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();

    emit purchaseOfferReceived(
        payload["cellId"].toInt(),
        payload["cellName"].toString(),
        payload["price"].toInt()
        );
}

void TcpClientController::handleAuctionUpdate(const QJsonObject& message)
{
    const QJsonObject payload = message["payload"].toObject();

    const int currentBid = payload["currentBid"].toInt();
    const int minimumBid = payload.contains("minimumBid")
                               ? payload["minimumBid"].toInt()
                               : currentBid + 1;

    emit auctionUpdated(
        payload["cellId"].toInt(),
        payload["cellName"].toString(),
        payload["secondsLeft"].toInt(),
        currentBid,
        minimumBid,
        payload["highestBidderName"].toString()
        );
}

void TcpClientController::handleAuctionFinished(const QJsonObject& message)
{
    Q_UNUSED(message);
    emit auctionFinished();
}
