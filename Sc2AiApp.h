#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Sc2AiAppDialog.h"

class Sc2AiManager;

class Sc2AiApp : public QMainWindow
{
    Q_OBJECT

public:
    Sc2AiApp(Sc2AiManager* MainApp, QWidget* parent = Q_NULLPTR);

    void InitalizeValues();

    std::string GenerateLadderConfig();


private:
    Ui::Sc2AiAppDialog ui;
    Sc2AiManager* MainApp;

private slots:
    void OnBrowseButtonPressed();
    void OnGenerateConfigPressed();
    void OnTestPressed();
    void OnUploadPressed();
    void OnLaunchPressed();
    void OnHumanLaunchPressed();
	void tabSelected();
};
