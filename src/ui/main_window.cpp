#include "main_window.hpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), serverController_(nullptr)
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

    connect(loginWidget_, &LoginWidget::loginRequested, this, &MainWindow::showMenu);


    connect(menuWidget_, &MenuWidget::createGameRequested, this, &MainWindow::createGame);
    connect(menuWidget_, &MenuWidget::joinGameRequested,   this, &MainWindow::joinGame);

    connect(lobbyWidget_, &LobbyWidget::startRequested, this, &MainWindow::showGame);

    //Network
    clientController_ = new TcpClientController(this);
    //connect(clientController_, &TcpClientController::lobbyUpdated, lobbyWidget_, &LobbyWidget::updatePlayers);
    connect(clientController_, &TcpClientController::connectedToServer, this, &MainWindow::sendConnectRequest);
}

MainWindow::~MainWindow() = default;

void MainWindow::sendConnectRequest()
{
    QJsonObject payload;
    payload["nickname"] = menuWidget_->nickname();

    clientController_->sendMessage(
        NetworkMessage::create(
            "connect_request",
            0,
            payload
            )
        );

    showLobby();
}

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

void MainWindow::createGame()
{
    if(!serverController_)
    {
        serverController_ = new TcpServerController(this);
        serverController_->startServer(7777);
    }
    clientController_->connectToServer("127.0.0.1",7777);
}

void MainWindow::joinGame()
{
    QString ip = menuWidget_->serverIp();
    clientController_->connectToServer(ip,7777);
}

