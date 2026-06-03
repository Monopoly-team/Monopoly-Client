#include "main_window.hpp"
#include <QLoggingCategory>

#include <QApplication>
#include <QFile>
#include <QTimer>

int main(int argc, char *argv[])
{
    QFile styleFile(":/resources/styles/main.qss");

    QLoggingCategory::setFilterRules(
        "qt.multimedia.*=false\n"
        "qt.multimedia.ffmpeg=false"
        );



    QApplication a(argc, argv);

    a.setWindowIcon(
        QIcon(":/resources/img/game.ico")
        );

    if(styleFile.open(QFile::ReadOnly))
        a.setStyleSheet(styleFile.readAll());

    MainWindow w;

    w.showMaximized();

    return a.exec();
}
