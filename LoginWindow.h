#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LoginWindow.h"

class Sc2AiManager;

class LoginWindow : public QMainWindow
{
	Q_OBJECT

public:
	LoginWindow(Sc2AiManager* MainApp, QWidget *parent = Q_NULLPTR);

private slots:
    void HandleLoginButton();

    void HandleSignupButton();

private:
	Ui::LoginWindowClass ui;
    Sc2AiManager* MainApp;

};
