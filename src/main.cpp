#include "main_window.hpp"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QFile styleFile(":/resources/styles/main.qss");
    QApplication a(argc, argv);

    if(styleFile.open(QFile::ReadOnly))
        a.setStyleSheet(styleFile.readAll());

    MainWindow w;
    w.showMaximized();

    return QCoreApplication::exec();
}
