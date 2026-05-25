#pragma once

#include <QMainWindow>
#include <QStackedWidget>

#include "menu_widget.hpp"
#include "lobby_widget.hpp"
#include "connect_widget.hpp"
#include "game_widget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void showMenu();
    void showConnect();
    void showLobby();
    void showGame();
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    QStackedWidget* screens_;
    MenuWidget*     menuWidget_;
    LobbyWidget*    lobbyWidget_;
    ConnectWidget*  connectWidget_;
    GameWidget*     gameWidget_;

};

