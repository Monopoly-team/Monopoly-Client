#include "network/client/tcp_client_controller.hpp"
#include "network/network_message.hpp"

#include <QDebug>

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
        const QJsonObject message = NetworkMessage::deserialize(line);

        if(!NetworkMessage::isValid(message))
        {
            qDebug() << "[Client] Invalid message from server" << line;
            continue;
        }
        qDebug() << "[Client] received " << message;
        emit messageReceived(message);
    }
}

void TcpClientController::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    qDebug() << "[Client] socket error: " << socket_->errorString();

    emit errorOccurred(socket_->errorString());
}


