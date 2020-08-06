#pragma once

#include <QtWidgets/QDialog>
#include "ui_ConfirmButton.h"

class Sc2AiManager;

class ConfirmButton : public QDialog
{
    Q_OBJECT

public:
    ConfirmButton(Sc2AiManager* MainApp, QWidget* parent = Q_NULLPTR);

    void ShowErrorMessage(std::string Message);

private:
    Ui::ConfirmButtonClass ui;
    Sc2AiManager* MainApp;

private slots:
    void OnOkButtonPressed();
};
