#include "main_window.hpp"
#include "network/server/tcp_server_controller.hpp"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QFile styleFile(":/resources/styles/main.qss");
    QApplication a(argc, argv);

    if(styleFile.open(QFile::ReadOnly))
        a.setStyleSheet(styleFile.readAll());

    MainWindow w;

    auto* server = new TcpServerController(&w);
    server->startServer(7777);


    w.showMaximized();



    return a.exec();
}
