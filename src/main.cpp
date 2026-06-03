#include "main_window.hpp"
#include "network/server/tcp_server_controller.hpp"
#include "network/client/tcp_client_controller.hpp"
#include "network/network_message.hpp"

#include <QApplication>
#include <QFile>
#include <QTimer>

int main(int argc, char *argv[])
{
    QFile styleFile(":/resources/styles/main.qss");
    QApplication a(argc, argv);

    if(styleFile.open(QFile::ReadOnly))
        a.setStyleSheet(styleFile.readAll());

    MainWindow w;

    //test net
/*    {
        auto* server = new TcpServerController(&w);
        server->startServer(7777);
        auto* client = new TcpClientController(&w);
        client->connectToServer("127.0.0.1",7777);
        auto* client2 = new TcpClientController(&w);
        client2->connectToServer("127.0.0.1",7777);
        QTimer::singleShot(2000,[client,client2](){
            QJsonObject payload;

            payload["nickname"] = "Ilja";
            client->sendMessage(NetworkMessage::create("connection_request",-1,payload));
            client2->sendMessage(NetworkMessage::create("connection_request",-1,payload));
        });
        QTimer::singleShot(4000,[client](){
            client->disconnectFromServer();
        });
    }*///end of testing net



    w.showMaximized();


    return a.exec();
}
