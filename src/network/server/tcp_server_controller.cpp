#include "network/server/tcp_server_controller.hpp"
#include "network/network_message.hpp"

#include <QHostAddress>
#include <QDebug>



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
    qDebug() << "[Server] Client accepted:"
             << QHostAddress(clientSocket->peerAddress().toIPv4Address()).toString()
             << clientSocket->peerPort();

    connect(clientSocket, &QTcpSocket::disconnected,this,[this,clientSocket](){
        qDebug() << "[Server] Client disconnected"
                 << QHostAddress(clientSocket->peerAddress().toIPv4Address()).toString()
                 << clientSocket->peerPort();
        clients_.removeOne(clientSocket);
        clientSocket->deleteLater();
    });
}

