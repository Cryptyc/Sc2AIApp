#include "LoginWindow.h"
#include "Sc2AiManager.h"
#include <shlobj.h>
#include <shlwapi.h>

LoginWindow::LoginWindow(Sc2AiManager* InMainApp, QWidget *parent)
	: QMainWindow(parent)
    , MainApp(InMainApp)
{
	ui.setupUi(this);
    ui.ErrorLabel->setText("");
    connect(ui.LoginButton, SIGNAL(released()), this, SLOT(HandleLoginButton()));
    connect(ui.SignUpButton, SIGNAL(released()), this, SLOT(HandleSignupButton()));
}

void LoginWindow::HandleLoginButton()
{
    std::string ErrorText;
    if (MainApp->LoginRequest(ui.UsernameEntry->text().toStdString(), ui.PasswordEntry->text().toStdString(), ErrorText))
    {
        MainApp->ShowMainDoalog();
    }
    else
    {
        std::string LabelText = std::string("<html><head/><body><p><span style=\" color:#ff0000;\">") + ErrorText + "</span></p></body></html>";
        ui.ErrorLabel->setText(QCoreApplication::translate("LoginWindowClass", LabelText.c_str(), nullptr));
    }
}

void LoginWindow::HandleSignupButton()
{
    ShellExecute(0, 0, L"https://sc2ai.net/signup.php", 0, 0, SW_SHOW);
}
