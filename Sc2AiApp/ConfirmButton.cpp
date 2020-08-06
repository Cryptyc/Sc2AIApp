#include "ConfirmButton.h"

ConfirmButton::ConfirmButton(Sc2AiManager* InMainApp, QWidget* parent)
    : QDialog(parent)
    , MainApp(InMainApp)
{
    ui.setupUi(this);
    connect(ui.OkButton, SIGNAL(released()), this, SLOT(OnOkButtonPressed()));
}

void ConfirmButton::ShowErrorMessage(std::string Message)
{
    ui.GenerateResult->setText(Message.c_str());
    show();
}

void ConfirmButton::OnOkButtonPressed()
{
    hide();
}
