#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <QVector>

class TcpServerController : public QObject
{

private slots:
    void onNewConnection();
public:
    TcpServerController(QObject* parent = nullptr);
    ~TcpServerController() override;

    bool startServer(quint16 port);

private:
    QTcpServer*             server_;
    QVector<QTcpSocket*>    clients_;
};

