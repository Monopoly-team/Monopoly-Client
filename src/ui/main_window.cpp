#include "main_window.hpp"
#include "network_constants.hpp"


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

    //Network + UI
    clientController_ = new TcpClientController(this);

    connect(clientController_, &TcpClientController::connectedToServer, this, &MainWindow::sendConnectRequest);
    connect(clientController_, &TcpClientController::lobbyUpdated,lobbyWidget_, &LobbyWidget::updatePlayers);
    connect(clientController_, &TcpClientController::gameStarted,this, &MainWindow::showGame);
    connect(lobbyWidget_, &LobbyWidget::readyChanged,this, &MainWindow::onReadyChanged);
    connect(clientController_, &TcpClientController::countdownUpdated,lobbyWidget_, &LobbyWidget::updateCountdown);
    connect(clientController_, &TcpClientController::countdownCancelled,lobbyWidget_, &LobbyWidget::cancelCountdown);
    connect(gameWidget_, &GameWidget::messageSent,this, &MainWindow::sendChatMessage);
    connect(clientController_, &TcpClientController::chatMessageReceived,this, &MainWindow::showChatMessage);
    connect(clientController_, &TcpClientController::gameEventReceived,this, &MainWindow::showGameEvent);
    connect(clientController_, &TcpClientController::gamePlayersUpdated, gameWidget_, &GameWidget::updatePlayers);
}



MainWindow::~MainWindow() = default;

void MainWindow::onReadyChanged(bool ready)
{

    QJsonObject payload;
    payload["ready"] = ready;

    clientController_->sendMessage(
        NetworkMessage::create(
            "ready_changed",
            clientController_->playerId(),
            payload
            )
        );
}

void MainWindow::sendChatMessage(const QString& text)
{
    QJsonObject payload;
    payload["text"] = text;

    clientController_->sendMessage(
        NetworkMessage::create(
            "chat_message",
            clientController_->playerId(),
            payload
            )
        );
}

void MainWindow::showChatMessage(const QString& nickname, const QString& text)
{
    gameWidget_->addEvent(
        QString("[%1] %2").arg(nickname, text)
        );
}

void MainWindow::showGameEvent(const QString& text)
{
    gameWidget_->addEvent("[Событие] " + text);
}

void MainWindow::sendConnectRequest()
{
    QJsonObject payload;
    payload["nickname"] = menuWidget_->nickname();

    clientController_->sendMessage(
        NetworkMessage::create(
            "connect_request",
            SERVER_ID,
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
    if (!serverController_)
        serverController_ = new TcpServerController(this);

    if (!serverController_->startServer(7777))
    {
        qDebug() << "[MainWindow] Cannot create game: server already running";
        return;
    }

    clientController_->connectToServer("127.0.0.1",7777);
}

void MainWindow::joinGame()
{
    QString ip = menuWidget_->serverIp();
    clientController_->connectToServer(ip,7777);
}

