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

#include "LaddermanagerTest.h"

using namespace rapidjson;
using namespace std;


Sc2AiApp::Sc2AiApp(Sc2AiManager* InMainApp, QWidget* parent)
    : QMainWindow(parent)
    , MainApp(InMainApp)
{
    ui.setupUi(this);
    connect(ui.BrowseButton, SIGNAL(released()), this, SLOT(OnBrowseButtonPressed()));
    connect(ui.ConfigButton, SIGNAL(released()), this, SLOT(OnGenerateConfigPressed()));
    connect(ui.TestButton, SIGNAL(released()), this, SLOT(OnTestPressed()));
    connect(ui.UploadButton, SIGNAL(released()), this, SLOT(OnUploadPressed()));
    connect(ui.LaunchButton, SIGNAL(released()), this, SLOT(OnLaunchPressed()));
    connect(ui.HLaunchButton, SIGNAL(released()), this, SLOT(OnHumanLaunchPressed()));
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

    for (const auto& ServerRegion : MainApp->GetServerRegions())
    {
        ui.RegionCombo->addItem(ServerRegion.first.c_str());
        ui.HRegionCombo->addItem(ServerRegion.first.c_str());
    }

    for (const std::string& Map : MainApp->GetActiveMaps())
    {
        ui.MapCombo->addItem(Map.c_str());
        ui.HMapCombo->addItem(Map.c_str());
    }
    for (const std::string LocalBot : MainApp->GetLocalBotDirs(true))
    {
        ui.LocalBotCombo->addItem(LocalBot.c_str());
    }

}

void Sc2AiApp::InitalizeValues()
{

}

void Sc2AiApp::OnBrowseButtonPressed()
{
//    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString fileName = QFileDialog::getOpenFileName(this);
    ui.DirectoryEdit->setText(fileName);
}

std::string Sc2AiApp::GenerateLadderConfig()
{
    Document ConfigDoc;
    ConfigDoc.SetObject();
    rapidjson::Document::AllocatorType& allocator = ConfigDoc.GetAllocator();
    rapidjson::Value BotsTopLevel(rapidjson::kObjectType);
    rapidjson::Value BotObject(rapidjson::kObjectType);

    std::string Race = ui.RaceCombo->currentText().toStdString();
    std::string API = ui.APICombo->currentText().toStdString();
    std::string FileName = ui.DirectoryEdit->text().toStdString();
    std::string BotName = ui.BotNameEdit->text().toStdString();
//    BotObject.AddMember(rapidjson::Value("BotName", allocator).Move(), rapidjson::Value(ui.BotNameEdit->text().toStdString().c_str(), allocator).Move(), allocator);
    BotObject.AddMember(rapidjson::Value("Race", allocator).Move(), rapidjson::Value(Race, allocator).Move(), allocator);
    BotObject.AddMember(rapidjson::Value("BotType", allocator).Move(), rapidjson::Value(API, allocator).Move(), allocator);
    BotObject.AddMember(rapidjson::Value("RootPath", allocator).Move(), rapidjson::Value("./", allocator).Move(), allocator);
    BotObject.AddMember(rapidjson::Value("FileName", allocator).Move(), rapidjson::Value(FileName, allocator).Move(), allocator);
    if (ui.DebugCheckBox->isChecked())
    {
        BotObject.AddMember(rapidjson::Value("Debug", allocator).Move(), rapidjson::Value(ui.DebugCheckBox->isChecked()), allocator);
    }
    string Arguments = ui.ArgsEdit->text().toStdString();
    if (Arguments != "")
    {
        BotObject.AddMember(rapidjson::Value("Args", allocator).Move(), rapidjson::Value(Arguments, allocator).Move(), allocator);
    }
    BotsTopLevel.AddMember(rapidjson::Value(BotName, allocator), BotObject, allocator);
    ConfigDoc.AddMember(rapidjson::Value("Bots", allocator), BotsTopLevel, allocator);
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    ConfigDoc.Accept(writer);
    std::string ReturnString = buffer.GetString();
    return ReturnString;
}

void Sc2AiApp::OnGenerateConfigPressed()
{
    string ConfigString = GenerateLadderConfig();
    string ConfigFileLocation = string(BOTS_DIRECTORY) + string("/") + ui.BotNameEdit->text().toStdString();
    std::filesystem::create_directories(ConfigFileLocation);

    ConfigFileLocation += string("/ladderbots.json");
    std::ofstream ofs(ConfigFileLocation);
    ofs << ConfigString;
    ofs.close();
}

void Sc2AiApp::OnTestPressed()
{
}

void Sc2AiApp::OnUploadPressed()
{
}

void Sc2AiApp::OnLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();

    sc2::Race NewRace = GetRaceFromString(ui.HRaceCombo->currentText().toStdString());

    NewGame->StartGame(ui.LocalBotCombo->currentText().toStdString(), ui.RemoteBotCombo->currentText().toStdString(), ui.MapCombo->currentText().toStdString(), "", "", ui.RegionCombo->currentText().toStdString(), false, NewRace);

}

void Sc2AiApp::OnHumanLaunchPressed()
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();

    sc2::Race NewRace = GetRaceFromString(ui.HRaceCombo->currentText().toStdString());

    NewGame->StartGame("Human", ui.HRemoteBotCombo->currentText().toStdString(), ui.HMapCombo->currentText().toStdString(), "", "", ui.HRegionCombo->currentText().toStdString(), true, NewRace);

}

void Sc2AiApp::tabSelected()
{
    if (ui.tabWidget->currentIndex() == 0)
    {

        // Do something here when user clicked at tab1

    }
    if (ui.tabWidget->currentIndex() == 3)
    {

        // Do something here when user clicked at tab4

    }
}
