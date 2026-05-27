#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class TcpClientController : public QObject
{
    Q_OBJECT
signals:
    void connectedToServer();
    void disconnectedFromServer();
    void messageReceived(const QJsonObject& message);
    void errorOccurred(const QString& error);
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
private:
    QTcpSocket* socket_;
};
