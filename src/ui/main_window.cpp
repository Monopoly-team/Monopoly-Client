#include "main_window.hpp"
#include "network_constants.hpp"


#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), serverController_(nullptr)
{

    audioService_ = new AudioService(this);
    audioService_->startBackgroundMusic();

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

    screens_->setCurrentWidget(menuWidget_);
    //screens_->setCurrentWidget(gameWidget_);


    settingsButton_ = new QPushButton("⚙", this);
    settingsButton_->setObjectName("settingsButton");
    settingsButton_->setFixedSize(52, 52);
    settingsButton_->raise();

    settingsDialog_ = new SettingsDialog(this);
    settingsDialog_->setMusicVolume(audioService_->musicVolume());

    connect(settingsButton_, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(settingsDialog_, &SettingsDialog::musicVolumeChanged, audioService_, &AudioService::setMusicVolume);

    updateSettingsButtonGeometry();

    clientController_ = new TcpClientController(this);

    connect(lobbyWidget_->readyButton(), &QPushButton::clicked, audioService_, &AudioService::playReadyClick);
    connect(menuWidget_->createButton(), &QPushButton::clicked, audioService_, &AudioService::playJoinSound);
    connect(menuWidget_->joinButton(),   &QPushButton::clicked, audioService_, &AudioService::playJoinSound);
    connect(clientController_,&TcpClientController::gameStarted,audioService_, &AudioService::playGameStart);

    connect(loginWidget_,      &LoginWidget::loginRequested,                    this, &MainWindow::showMenu);
    connect(menuWidget_,       &MenuWidget::createGameRequested,                this, &MainWindow::createGame);
    connect(menuWidget_,       &MenuWidget::joinGameRequested,                  this, &MainWindow::joinGame);
    connect(clientController_, &TcpClientController::serverDisconnectRequested, this, &MainWindow::onServerDisconnectRequested);
    connect(clientController_, &TcpClientController::connectedToServer,         this, &MainWindow::sendConnectRequest);
    connect(clientController_, &TcpClientController::lobbyUpdated,              this, &MainWindow::onLobbyUpdated);
    connect(clientController_, &TcpClientController::gameStarted,               this, &MainWindow::showGame);
    connect(lobbyWidget_,      &LobbyWidget::readyChanged,                      this, &MainWindow::onReadyChanged);
    connect(clientController_, &TcpClientController::countdownUpdated,          lobbyWidget_, &LobbyWidget::updateCountdown);
    connect(clientController_, &TcpClientController::countdownCancelled,        lobbyWidget_, &LobbyWidget::cancelCountdown);
    connect(gameWidget_,       &GameWidget::messageSent,                        this, &MainWindow::sendChatMessage);
    connect(gameWidget_,       &GameWidget::rollDiceRequested,                  this, &MainWindow::sendRollDiceAction);
    connect(gameWidget_,       &GameWidget::rollDiceRequested,                  audioService_, &AudioService::playDiceSound);
    connect(gameWidget_,       &GameWidget::buildBusinessRequested,             this, &MainWindow::sendBuildBusinessAction);
    connect(clientController_, &TcpClientController::purchaseOfferReceived,     this, &MainWindow::showPurchaseOffer);
    connect(clientController_, &TcpClientController::auctionUpdated,            this, &MainWindow::showAuctionUpdate);
    connect(clientController_, &TcpClientController::auctionFinished,           this, &MainWindow::closeAuctionDialog);
    connect(clientController_, &TcpClientController::chatMessageReceived,       this, &MainWindow::showChatMessage);
    connect(clientController_, &TcpClientController::gameEventReceived,         this, &MainWindow::showGameEvent);
    connect(clientController_, &TcpClientController::gameStateUpdated,          gameWidget_, &GameWidget::updateGameState);
    connect(clientController_, &TcpClientController::gameStateUpdated,          this, &MainWindow::updateLocalPlayerState);
    connect(clientController_, &TcpClientController::errorOccurred,             this, &MainWindow::showNetworkError);
    connect(clientController_, &TcpClientController::disconnectedFromServer,    this, &MainWindow::handleServerDisconnected);
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

void MainWindow::onLobbyUpdated(const QVector<ClientLobbyPlayer>& players)
{
    if (players.size() > lastLobbyPlayersCount_ && lastLobbyPlayersCount_ > 0)
        audioService_->playJoinSound();

    lastLobbyPlayersCount_ = players.size();

    lobbyWidget_->updatePlayers(players);
}

void MainWindow::onServerDisconnectRequested(const QString& reason)
{
    QString text = "Соединение закрыто сервером";

    if(reason == "kicked_by_admin")
    {
        text = "Вы были отключены администратором";
    }

    QMessageBox::warning(
        this,
        "Отключение",
        text
        );

    showMenu();
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

void MainWindow::showChatMessage(quint16 playerId, const QString& nickname, const QString& text)
{
    gameWidget_->addChatMessage(playerId, nickname, text);
}

void MainWindow::showNetworkError(const QString& message)
{
    QMessageBox::warning(
        this,
        "Ошибка подключения",
        message.isEmpty() ? "Произошла ошибка сети" : message
        );
}

void MainWindow::handleServerDisconnected()
{
    if (screens_->currentWidget() == loginWidget_)
        return;

    QMessageBox::warning(
        this,
        "Соединение потеряно",
        "Подключение к серверу было закрыто."
        );

    showMenu();
}

void MainWindow::showGameEvent(const QString& text)
{
    gameWidget_->addEvent(text);
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
    gameWidget_->setLocalPlayerId(clientController_->playerId());

    screens_->setCurrentWidget(gameWidget_);
}

void MainWindow::createGame()
{
    if (!ensureNicknameEntered())
        return;
    if (!serverController_)
        serverController_ = new TcpServerController(this);

    if (!serverController_->startServer(7777))
    {
        if (serverController_ && !serverController_->isListening())
        {
            serverController_->deleteLater();
            serverController_ = nullptr;
        }
        qDebug() << "[MainWindow] Cannot create game: server already running";
        QMessageBox::warning(
            this,
            "Ошибка создания сервера",
            "Сервер уже запущен на этом порту."
            );
        return;
    }

    clientController_->connectToServer("127.0.0.1",7777);
}

void MainWindow::joinGame()
{
    if (!ensureNicknameEntered())
        return;
    QString ip = menuWidget_->serverIp();
    clientController_->connectToServer(ip,7777);
}

void MainWindow::openSettings()
{
    settingsDialog_->setMusicVolume(audioService_->musicVolume());
    settingsDialog_->exec();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateSettingsButtonGeometry();
}

void MainWindow::updateSettingsButtonGeometry()
{
    const int margin = 24;

    settingsButton_->move(
        width() - settingsButton_->width() - margin,
        margin
        );

    settingsButton_->raise();
}

void MainWindow::sendRollDiceAction()
{
    if (gameOver_)
        return;
    QJsonObject payload;
    payload["action"] = "roll_dice";

    clientController_->sendMessage(
        NetworkMessage::create(
            "player_action",
            clientController_->playerId(),
            payload
            )
        );
}

void MainWindow::showPurchaseOffer(int cellId, const QString& cellName, int price)
{
    if (gameOver_)
        return;
    if (!purchaseOfferDialog_)
    {
        purchaseOfferDialog_ = new PurchaseOfferDialog(this);

        connect(purchaseOfferDialog_, &PurchaseOfferDialog::buyRequested,
                this, &MainWindow::sendBuyBusinessAction);
        connect(purchaseOfferDialog_, &PurchaseOfferDialog::buyRequested,
                audioService_, &AudioService::playBuySound);

        connect(purchaseOfferDialog_, &PurchaseOfferDialog::auctionRequested,
                this, &MainWindow::sendStartAuctionAction);
        connect(purchaseOfferDialog_, &PurchaseOfferDialog::auctionRequested,
                audioService_, &AudioService::playAuctionStartSound);
    }

    purchaseOfferDialog_->setOffer(cellId, cellName, price, localPlayerBalance_);
    purchaseOfferDialog_->show();
    purchaseOfferDialog_->raise();
    purchaseOfferDialog_->activateWindow();
}

void MainWindow::showAuctionUpdate(
    int cellId,
    const QString& cellName,
    int secondsLeft,
    int currentBid,
    int minimumBid,
    const QString& highestBidderName
    )
{
    if (gameOver_)
        return;
    if (!auctionDialog_)
    {
        auctionDialog_ = new AuctionDialog(this);

        connect(auctionDialog_, &AuctionDialog::bidRequested,
                this, &MainWindow::sendAuctionBidAction);
    }

    const bool shouldShowDialog = !auctionDialog_->isVisible();

    auctionDialog_->updateAuction(
        cellId,
        cellName,
        secondsLeft,
        currentBid,
        minimumBid,
        highestBidderName
        );

    if (shouldShowDialog)
    {
        auctionDialog_->show();
        auctionDialog_->raise();
        auctionDialog_->activateWindow();
    }

    gameWidget_->setAuctionActive(true);
}

void MainWindow::closeAuctionDialog()
{
    if (auctionDialog_)
        auctionDialog_->hide();

    if (purchaseOfferDialog_)
        purchaseOfferDialog_->hide();

    gameWidget_->setAuctionActive(false);
}

void MainWindow::sendBuyBusinessAction()
{
    if (gameOver_)
        return;
    QJsonObject payload;
    payload["action"] = "buy_business";

    clientController_->sendMessage(
        NetworkMessage::create(
            "player_action",
            clientController_->playerId(),
            payload
            )
        );
}

void MainWindow::updateLocalPlayerState(const ClientGameState& state)
{
    gameOver_ = state.isGameOver;

    if (gameOver_)
    {
        if (purchaseOfferDialog_)
            purchaseOfferDialog_->hide();

        if (auctionDialog_)
            auctionDialog_->hide();

        gameWidget_->setAuctionActive(false);
    }

    const quint16 localPlayerId = clientController_->playerId();

    for (const ClientGamePlayer& player : state.players)
    {
        if (player.id == localPlayerId)
        {
            localPlayerBalance_ = player.balance;
            return;
        }
    }

    localPlayerBalance_ = 0;
}

void MainWindow::sendStartAuctionAction()
{
    if (gameOver_)
        return;
    QJsonObject payload;
    payload["action"] = "start_auction";

    clientController_->sendMessage(
        NetworkMessage::create(
            "player_action",
            clientController_->playerId(),
            payload
            )
        );
}

void MainWindow::sendAuctionBidAction(int amount)
{
    if (gameOver_)
        return;
    QJsonObject payload;
    payload["action"] = "auction_bid";
    payload["amount"] = amount;

    clientController_->sendMessage(
        NetworkMessage::create(
            "player_action",
            clientController_->playerId(),
            payload
            )
        );
}

void MainWindow::sendBuildBusinessAction(int cellId)
{
    if (gameOver_)
        return;
    QJsonObject payload;
    payload["action"] = "build_business";
    payload["cellId"] = cellId;

    clientController_->sendMessage(
        NetworkMessage::create(
            "player_action",
            clientController_->playerId(),
            payload
            )
        );
}

bool MainWindow::ensureNicknameEntered()
{
    if (menuWidget_->hasNickname())
        return true;

    QMessageBox::warning(
        this,
        "Никнейм не указан",
        "Введите никнейм перед созданием игры или подключением."
        );

    return false;
}