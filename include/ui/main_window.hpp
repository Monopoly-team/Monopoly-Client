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
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    QStackedWidget* screens_;
    LoginWidget*    loginWidget_;
    LobbyWidget*    lobbyWidget_;
    MenuWidget*     menuWidget_;
    GameWidget*     gameWidget_;

    TcpClientController* clientController_;
    TcpServerController* serverController_ = nullptr;

};

