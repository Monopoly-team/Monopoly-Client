#include "main_window.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    screens_        = new QStackedWidget(this);

    loginWidget_    = new LoginWidget(this);
    menuWidget_     = new MenuWidget(this);
    lobbyWidget_    = new LobbyWidget(this);
    gameWidget_     = new GameWidget(this);

    screens_->addWidget(loginWidget_);
    screens_->addWidget(menuWidget_);
    screens_->addWidget(lobbyWidget_);
    screens_->addWidget(gameWidget_);

    setCentralWidget(screens_);

    screens_->setCurrentWidget(loginWidget_);
}

MainWindow::~MainWindow() = default;

void MainWindow::showLogin()
{
    screens_->setCurrentWidget(loginWidget_);
}
void MainWindow::showMenu()
{
    screens_->setCurrentWidget(menuWidget_);
}
void MainWindow::showLobby()
{
    screens_->setCurrentWidget(lobbyWidget_);
}
void MainWindow::showGame()
{
    screens_->setCurrentWidget(gameWidget_);
}

