#pragma once

#include <QMainWindow>
#include <QStackedWidget>

#include "login_widget.hpp"
#include "lobby_widget.hpp"
#include "menu_widget.hpp"
#include "game_widget.hpp"

#include "network/client/tcp_client_controller.hpp"
#include "network/server/tcp_server_controller.hpp"
#include "network/network_message.hpp"
#include "services/audio_service.hpp"
#include "ui/settings_dialog.hpp"
#include "ui/purchase_offer_dialog.hpp"
#include "ui/auction_dialog.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void showLogin();
    void showMenu();
    void showLobby();
    void showGame();

    void createGame();
    void joinGame();
    void sendConnectRequest();
    void onReadyChanged(bool ready);
    void onLobbyUpdated(const QVector<ClientLobbyPlayer>& players);
    void sendChatMessage(const QString& text);
    void showGameEvent(const QString& text);
    void showChatMessage(const QString &nickname, const QString &text);
    void showNetworkError(const QString& message);
    void handleServerDisconnected();
    void onServerDisconnectRequested(const QString &reason);
    void openSettings();
    void updateSettingsButtonGeometry();
    void sendRollDiceAction();
    void showPurchaseOffer(int cellId, const QString& cellName, int price);
    void showAuctionUpdate(int cellId, const QString& cellName, int secondsLeft, int currentBid, const QString& highestBidderName);
    void closeAuctionDialog();
    void sendBuyBusinessAction();
    void updateLocalPlayerState(const ClientGameState& state);
    void sendStartAuctionAction();
    void sendAuctionBidAction(int amount);
    void sendBuildBusinessAction(int cellId);
protected:
    void resizeEvent(QResizeEvent* event) override;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    int lastLobbyPlayersCount_ = 0;

    QStackedWidget* screens_;
    LoginWidget*    loginWidget_;
    LobbyWidget*    lobbyWidget_;
    MenuWidget*     menuWidget_;
    GameWidget*     gameWidget_;
    AudioService*   audioService_;
    QPushButton*    settingsButton_;
    SettingsDialog* settingsDialog_;

    TcpClientController* clientController_;
    TcpServerController* serverController_      = nullptr;
    PurchaseOfferDialog* purchaseOfferDialog_   = nullptr;
    AuctionDialog*       auctionDialog_         = nullptr;
    int                  localPlayerBalance_    = 0;

};

