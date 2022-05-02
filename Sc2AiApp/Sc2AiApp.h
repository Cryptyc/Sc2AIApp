#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Sc2AiAppDialog.h"

class Sc2AiManager;

class Sc2AiApp : public QMainWindow
{
    Q_OBJECT

public:
    Sc2AiApp(Sc2AiManager* MainApp, QWidget* parent = Q_NULLPTR);

    // Input field getters
    std::string GetRace() { return ""; }
    std::string GetAPI() { return ""; }
    std::string GetFileName() { return ""; }
    std::string GetBotName() { return ""; }
    std::string GetArgs() { return ""; }
    bool GetDebug() { return false; }
    bool GetDowloadable() { return false; }
    bool GetHumanPlayable() { return false; }

private:
    Ui::Sc2AiAppDialog ui;
    Sc2AiManager* MainApp;
    std::vector<std::string> BotDirectories;
    std::vector<std::string> ActiveMaps;
    std::string BotsDirectory = "Bots/";

private slots:
    void OnBrowseButtonPressed();
    void OnGenerateConfigPressed();
    void OnUploadPressed();
    void OnLaunchPressed();
    void OnHumanLaunchPressed();
    void OnArchonLaunchPressed();
    void tabSelected();
    void LocalPlayChecked(int checkIndex);
};
