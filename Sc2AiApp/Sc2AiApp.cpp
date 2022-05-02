#include "Sc2AiApp.h"
#include "Sc2AiManager.h"
#include <QtWidgets/qfiledialog.h>

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "LadderManagerTest.h"
#include <algorithm>

using namespace rapidjson;
using namespace std;


#include <string>



Sc2AiApp::Sc2AiApp(Sc2AiManager* InMainApp, QWidget* parent)
    : QMainWindow(parent)
    , MainApp(InMainApp)
{
    ui.setupUi(this);
    connect(ui.BrowseButton, SIGNAL(released()), this, SLOT(OnBrowseButtonPressed()));
    connect(ui.HBrowseButton, SIGNAL(released()), this, SLOT(OnBrowseButtonPressed()));
    connect(ui.LaunchButton, SIGNAL(released()), this, SLOT(OnLaunchPressed()));
    connect(ui.HLaunchButton, SIGNAL(released()), this, SLOT(OnHumanLaunchPressed()));
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

    MainApp->GetActiveMaps(ActiveMaps);
    for (const std::string& Map : ActiveMaps)
    {
        ui.MapCombo->addItem(Map.c_str());
        ui.HMapCombo->addItem(Map.c_str());
    }
    MainApp->GetLocalBotDirs("", true, BotDirectories);
    for (const std::string LocalBot : BotDirectories)
    {
        ui.LocalBotCombo->addItem(LocalBot.c_str());
        ui.RemoteBotCombo->addItem(LocalBot.c_str());
        ui.HRemoteBotCombo->addItem(LocalBot.c_str());
    }
}

void Sc2AiApp::OnBrowseButtonPressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    BotsDirectory = dir.toStdString();
    std::replace(BotsDirectory.begin(), BotsDirectory.end(), '/', '\\'); // replace all 'x' to 'y'
    BotDirectories.clear();
    ui.LocalBotText->setText(BotsDirectory.c_str());
    ui.HLocalBotText->setText(BotsDirectory.c_str());
    ui.LocalBotCombo->clear();
    ui.RemoteBotCombo->clear();
    ui.HRemoteBotCombo->clear();
    MainApp->GetLocalBotDirs("", true, BotDirectories);
    for (const std::string LocalBot : BotDirectories)
    {
        ui.LocalBotCombo->addItem(LocalBot.c_str());
        ui.RemoteBotCombo->addItem(LocalBot.c_str());
        ui.HRemoteBotCombo->addItem(LocalBot.c_str());
    }
}

void Sc2AiApp::OnGenerateConfigPressed()
{
    std::string Error;
    MainApp->WriteBotConfig(Error);
    MainApp->ShowError(Error);
}

void Sc2AiApp::OnUploadPressed()
{
    std::string Error;
    if (!MainApp->WriteBotConfig(Error))
    {
        MainApp->ShowError(Error);
    }
    std::string BotZipLocation = "";
    if (BotZipLocation != "")
    {
//        MainApp->UploadBot(ui.BotNameEdit->text().toStdString(), ui.RaceCombo->currentText().toStdString(), BotZipLocation, true, Error);
        MainApp->ShowError(Error);
    }
}

void Sc2AiApp::OnLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();
    if (ui.LocalBotText->text().toStdString().length() > 0)
    {
        NewGame->SetBotDirectory("");
    }

    sc2::Race NewRace = GetRaceFromString(ui.HRaceCombo->currentText().toStdString());

    GameResult result = NewGame->StartGame(ui.LocalBotCombo->currentText().toStdString(), ui.RemoteBotCombo->currentText().toStdString(), ui.MapCombo->currentText().toStdString(), MainApp->ServerUsername, MainApp->ServerToken, false, false, false, NewRace);

}

void Sc2AiApp::OnHumanLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();
    std::string LocalBotName = ui.HRemoteBotCombo->currentText().toStdString();
    if (LocalBotName.length() > 0)
    {
        NewGame->SetBotDirectory(BotsDirectory);
    }
    std::string BotRace = ui.HRaceCombo->currentText().toStdString();

    sc2::Race NewRace = GetRaceFromString(BotRace);
    std::string MapName = ui.HMapCombo->currentText().toStdString();
    std::string HumanName = ui.HumanNameText->text().toStdString();
    GameResult result = NewGame->StartGame(HumanName, LocalBotName, MapName, MainApp->ServerUsername, MainApp->ServerToken, true, false, true, NewRace);

}

void Sc2AiApp::OnArchonLaunchPressed()
{

}


void Sc2AiApp::tabSelected()
{
    MainApp->ToggleBotsListWindow(ui.tabWidget->currentIndex() == 0);
}

void Sc2AiApp::LocalPlayChecked(int checkIndex)
{
    ui.RemoteBotCombo->clear();
    ui.HRemoteBotCombo->clear();
    if (checkIndex)
    {
        MainApp->GetLocalBotDirs("", true, BotDirectories);
        for (const std::string LocalBot : BotDirectories)
        {
            ui.RemoteBotCombo->addItem(LocalBot.c_str());
            ui.HRemoteBotCombo->addItem(LocalBot.c_str());
        }

    }
    else
    {
        for (const std::string& RemoteBot : MainApp->GetRemoteBotList(TESTABLE_BOTS_PATH))
        {
            ui.RemoteBotCombo->addItem(RemoteBot.c_str());
        }
        for (const std::string& HumanRemoteBot : MainApp->GetRemoteBotList(HUMAN_BOTS_PATH))
        {
            ui.HRemoteBotCombo->addItem(HumanRemoteBot.c_str());
        }
    }
}
