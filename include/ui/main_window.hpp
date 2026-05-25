#pragma once

#include <QMainWindow>
#include <QStackedWidget>

#include "login_widget.hpp"
#include "lobby_widget.hpp"
#include "menu_widget.hpp"
#include "game_widget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void showLogin();
    void showMenu();
    void showLobby();
    void showGame();
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    QStackedWidget* screens_;
    LoginWidget*    loginWidget_;
    LobbyWidget*    lobbyWidget_;
    MenuWidget*     menuWidget_;
    GameWidget*     gameWidget_;

};

