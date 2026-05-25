#include "main_window.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    screens_       = new QStackedWidget(this);

    menuWidget_    = new MenuWidget(this);
    connectWidget_ = new ConnectWidget(this);
    lobbyWidget_   = new LobbyWidget(this);
    gameWidget_    = new GameWidget(this);

    screens_->addWidget(menuWidget_);
    screens_->addWidget(connectWidget_);
    screens_->addWidget(lobbyWidget_);
    screens_->addWidget(gameWidget_);

    setCentralWidget(screens_);

    screens_->setCurrentWidget(menuWidget_);
}

MainWindow::~MainWindow() = default;

void MainWindow::showMenu()
{
    screens_->setCurrentWidget(menuWidget_);
}
void MainWindow::showConnect()
{
    screens_->setCurrentWidget(connectWidget_);
}
void MainWindow::showLobby()
{
    screens_->setCurrentWidget(lobbyWidget_);
}
void MainWindow::showGame()
{
    screens_->setCurrentWidget(gameWidget_);
}

