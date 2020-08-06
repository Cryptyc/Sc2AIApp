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

using namespace rapidjson;
using namespace std;


Sc2AiApp::Sc2AiApp(Sc2AiManager* InMainApp, QWidget* parent)
    : QMainWindow(parent)
    , MainApp(InMainApp)
{
    ui.setupUi(this);
    connect(ui.BrowseButton, SIGNAL(released()), this, SLOT(OnBrowseButtonPressed()));
    connect(ui.ABrowseButton, SIGNAL(released()), this, SLOT(OnBrowseButtonPressed()));
    connect(ui.UBrowseButton, SIGNAL(released()), this, SLOT(OnBrowseButtonPressed()));
    connect(ui.ConfigButton, SIGNAL(released()), this, SLOT(OnGenerateConfigPressed()));
    connect(ui.UploadButton, SIGNAL(released()), this, SLOT(OnUploadPressed()));
    connect(ui.LaunchButton, SIGNAL(released()), this, SLOT(OnLaunchPressed()));
    connect(ui.HLaunchButton, SIGNAL(released()), this, SLOT(OnHumanLaunchPressed()));
    connect(ui.ALaunchButton, SIGNAL(released()), this, SLOT(OnArchonLaunchPressed()));
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));
    connect(ui.TestLocalCheck, SIGNAL(stateChanged(int)), this, SLOT(LocalPlayChecked(int)));
    connect(ui.PlayLocalCheck, SIGNAL(stateChanged(int)), this, SLOT(LocalPlayChecked(int)));
    connect(ui.ArchonLocalCheck, SIGNAL(stateChanged(int)), this, SLOT(LocalPlayChecked(int)));


    for (const std::string& Map : MainApp->GetActiveMaps())
    {
        ui.MapCombo->addItem(Map.c_str());
        ui.HMapCombo->addItem(Map.c_str());
        ui.AMapCombo->addItem(Map.c_str());
    }
    for (const std::string LocalBot : MainApp->GetLocalBotDirs("",  true))
    {
        ui.LocalBotCombo->addItem(LocalBot.c_str());
        ui.ArchonCombo->addItem(LocalBot.c_str());
    }

    for (const std::string& RemoteBot : MainApp->GetRemoteBotList(TESTABLE_BOTS_PATH))
    {
        ui.RemoteBotCombo->addItem(RemoteBot.c_str());
    }

    for (const std::string& HumanRemoteBot : MainApp->GetRemoteBotList(HUMAN_BOTS_PATH))
    {
        ui.HRemoteBotCombo->addItem(HumanRemoteBot.c_str());
        ui.ARemoteBotCombo->addItem(HumanRemoteBot.c_str());
    }
}

void Sc2AiApp::OnBrowseButtonPressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui.ULocalBotText->setText(dir);
    ui.ALocalBotText->setText(dir);
    ui.LocalBotText->setText(dir);
    ui.LocalBotCombo->clear();
    ui.ArchonCombo->clear();
    for (const std::string LocalBot : MainApp->GetLocalBotDirs(dir.toStdString(), true))
    {
        ui.LocalBotCombo->addItem(LocalBot.c_str());
        ui.ArchonCombo->addItem(LocalBot.c_str());
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
    std::string BotZipLocation = MainApp->PrepareBotDirectory(ui.BotNameEdit->text().toStdString(), ui.RaceCombo->currentText().toStdString(), Error);
    if (BotZipLocation != "")
    {
        MainApp->UploadBot(ui.BotNameEdit->text().toStdString(), ui.RaceCombo->currentText().toStdString(), BotZipLocation, true, Error);
        MainApp->ShowError(Error);
    }
}

void Sc2AiApp::OnLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();
    if (ui.LocalBotText->text().toStdString().length() > 0)
    {
        NewGame->SetBotDirectory(ui.ALocalBotText->text().toStdString());
    }

    sc2::Race NewRace = GetRaceFromString(ui.HRaceCombo->currentText().toStdString());

    GameResult result = NewGame->StartGame(ui.LocalBotCombo->currentText().toStdString(), ui.RemoteBotCombo->currentText().toStdString(), ui.MapCombo->currentText().toStdString(), MainApp->ServerUsername, MainApp->ServerToken, false, !ui.TestLocalCheck->isChecked(), false, NewRace);

}

void Sc2AiApp::OnHumanLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();
    if (ui.ULocalBotText->text().toStdString().length() > 0)
    {
        NewGame->SetBotDirectory(ui.ULocalBotText->text().toStdString());
    }

    sc2::Race NewRace = GetRaceFromString(ui.HRaceCombo->currentText().toStdString());

    GameResult result = NewGame->StartGame("Human", ui.HRemoteBotCombo->currentText().toStdString(), ui.HMapCombo->currentText().toStdString(), MainApp->ServerUsername, MainApp->ServerToken, true, !ui.PlayLocalCheck->isChecked(), true, NewRace);

}

void Sc2AiApp::OnArchonLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();
    if (ui.ALocalBotText->text().toStdString().length() > 0)
    {
        NewGame->SetBotDirectory(ui.ALocalBotText->text().toStdString());
    }

    GameResult result = NewGame->StartGame(ui.ArchonCombo->currentText().toStdString(), ui.ARemoteBotCombo->currentText().toStdString(), ui.HMapCombo->currentText().toStdString(), MainApp->ServerUsername, MainApp->ServerToken, false, !ui.ArchonLocalCheck->isChecked(), true);

}


void Sc2AiApp::tabSelected()
{
    MainApp->ToggleBotsListWindow(ui.tabWidget->currentIndex() == 0);
}

void Sc2AiApp::LocalPlayChecked(int checkIndex)
{
    ui.RemoteBotCombo->clear();
    ui.ARemoteBotCombo->clear();
    ui.HRemoteBotCombo->clear();
    if (checkIndex)
    {
        for (const std::string LocalBot : MainApp->GetLocalBotDirs("", true))
        {
            ui.RemoteBotCombo->addItem(LocalBot.c_str());
            ui.HRemoteBotCombo->addItem(LocalBot.c_str());
            ui.ARemoteBotCombo->addItem(LocalBot.c_str());
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
            ui.ARemoteBotCombo->addItem(HumanRemoteBot.c_str());
        }
    }
}
