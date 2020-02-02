#include <vector>
#include <string>
#include <qmessagebox.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include "Tools.h"

#include "BotsList.h"
#include "Sc2AiManager.h"

using namespace std;

BotsList::BotsList(Sc2AiManager* InMainApp, QWidget* parent)
    : QMainWindow(parent)
    , MainApp(InMainApp)
{
    ui.setupUi(this);
    connect(ui.RefreshButton, SIGNAL(released()), this, SLOT(OnRefreshButtonPressed()));

}

std::string BotsList::GetBotStatus(const BotData& Bot)
{
    if (Bot.Rejected)
    {
        return "Rejected";
    }
    if (Bot.Deleted)
    {
        return "Deleted";
    }
    if (Bot.Deactivated || (Bot.BotPlayable == false && Bot.HumanPlayable == false))
    {
        return "Deativated";
    }
    if (Bot.Verified)
    {
        return "Active";
    }
    return "Awaiting verification";
}


void BotsList::OnRefreshButtonPressed()
{
//    PrintWidth();
    vector<string> arguments;
    std::string argument = " -F Username=" + MainApp->ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + MainApp->ServerToken;
    arguments.push_back(argument);
    std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
    std::string ErrorString;
    MainApp->RefreshBotsList(ListResult, ErrorString);
}

void BotsList::RefreshList(std::map<std::string, BotData> ServerBots)
{
    int RowCount = ServerBots.size();
    ui.BotsListTable->setRowCount(0);
    ui.BotsListTable->setRowCount(ServerBots.size());
    std::map<std::string, BotData>::iterator it = ServerBots.begin();

    int CurrentRow = 0;
    while (it != ServerBots.end())
    {
        BotData CurrentBot = it->second;
        if (CurrentBot.Deleted)
        {
            it++;
            continue;
        }
        ui.BotsListTable->setItem(CurrentRow, 0, new QTableWidgetItem(CurrentBot.BotName.c_str()));
        ui.BotsListTable->setItem(CurrentRow, 1, new QTableWidgetItem(GetRaceString(CurrentBot.Race).c_str()));
        if ((!CurrentBot.Rejected) && CurrentBot.Verified)
        {
            QPushButton* btn_enabled = new QPushButton();
            if (CurrentBot.Deactivated)
            {
                btn_enabled->setText("Disabled");
            }
            else
            {
                btn_enabled->setText("Enabled");

            }
            ui.BotsListTable->setCellWidget(CurrentRow, 2, (QWidget*)btn_enabled);
            connect(btn_enabled, &QPushButton::released, [this, BotId = CurrentBot.Id, Enabled = CurrentBot.Deactivated]{ ToggleDeactivated(BotId, Enabled); });

        }
        else
        {
            ui.BotsListTable->setItem(CurrentRow, 2, new QTableWidgetItem(GetBotStatus(CurrentBot).c_str()));
        }
        QPushButton* btn_vsbot = new QPushButton();
        if (CurrentBot.BotPlayable)
        {
            btn_vsbot->setText("Enabled");
        }
        else
        {
            btn_vsbot->setText("Disabled");

        }
        ui.BotsListTable->setCellWidget(CurrentRow, 3, (QWidget*)btn_vsbot);
        QPushButton* btn_vshuman = new QPushButton();
        if (CurrentBot.HumanPlayable)
        {
            btn_vshuman->setText("Enabled");
        }
        else
        {
            btn_vshuman->setText("Disabled");

        }
        ui.BotsListTable->setCellWidget(CurrentRow, 4, (QWidget*)btn_vshuman);
        QPushButton* btn_downloadble = new QPushButton();
        if (CurrentBot.Downloadable)
        {
            btn_downloadble->setText("Enabled");
        }
        else
        {
            btn_downloadble->setText("Disabled");

        }
        ui.BotsListTable->setCellWidget(CurrentRow, 5, (QWidget*)btn_downloadble);
        QPushButton* btn_update = new QPushButton();
        btn_update->setText("Update");
        ui.BotsListTable->setCellWidget(CurrentRow, 6, (QWidget*)btn_update);
        QPushButton* btn_delete = new QPushButton();
        btn_delete->setText("Delete");
        ui.BotsListTable->setCellWidget(CurrentRow, 7, (QWidget*)btn_delete);
        QPushButton* btn_download = new QPushButton();
        btn_download->setText("Download");
        ui.BotsListTable->setCellWidget(CurrentRow, 8, (QWidget*)btn_download);
        connect(btn_vsbot, &QPushButton::released, [this, BotId = CurrentBot.Id, BotPlayable = CurrentBot.BotPlayable] { ToggleVsBot(BotId, BotPlayable); });
        connect(btn_vshuman, &QPushButton::released, [this, BotId = CurrentBot.Id, HumanPlayable = CurrentBot.HumanPlayable] { ToggleVsHuman(BotId, HumanPlayable); });
        connect(btn_downloadble, &QPushButton::released, [this, BotId = CurrentBot.Id, Downloadable = CurrentBot.Downloadable]{ ToggleDownloadable(BotId, Downloadable); });
        connect(btn_update, &QPushButton::released, [this, BotName = CurrentBot.BotName, BotRace = CurrentBot.Race] { UpdateBot(BotName, GetRaceString(BotRace)); });
        connect(btn_delete, &QPushButton::released, [this, BotId = CurrentBot.Id] { DeleteBot(BotId); });
        connect(btn_download, &QPushButton::released, [this, BotId = CurrentBot.Id, BotName = CurrentBot.BotName]{ DownloadData(BotId, BotName); });

        it++;
        CurrentRow++;
    }
}

void BotsList::DownloadData(const int BotId, const std::string& BotName)
{
    std::vector<std::string> arguments;
    std::string argument = " -F Username=" + MainApp->ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + MainApp->ServerToken;
    arguments.push_back(argument);
    argument = " -F BotID=" + std::to_string(BotId);
    arguments.push_back(argument);

    std::string BotZipLocation = MainApp->CurrentBotDir + "/" + BotName + ".zip";
    remove(BotZipLocation.c_str());
    argument = " -o " + BotZipLocation;
    arguments.push_back(argument);
    PerformRestRequest(DATA_DOWNLOAD_PATH, arguments);
    MainApp->ShowError("Bot data saved to " + BotZipLocation);

    
}

void BotsList::DeleteBot(const int BotId)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Really delete Bot", "Are you really sure you want to delete this bot?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        vector<string> arguments;
        std::string argument = " -F Username=" + MainApp->ServerUsername;
        arguments.push_back(argument);
        argument = " -F Token=" + MainApp->ServerToken;
        arguments.push_back(argument);
        argument = std::string(" -F DeleteBotID=") + std::to_string(BotId);
        arguments.push_back(argument);
        std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
        std::string ErrorString;
        MainApp->RefreshBotsList(ListResult, ErrorString);
    }
}

void BotsList::ToggleVsHuman(const int BotId, bool Enable)
{
    vector<string> arguments;
    std::string argument = " -F Username=" + MainApp->ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + MainApp->ServerToken;
    arguments.push_back(argument);
    if (Enable)
    {
        argument = std::string(" -F DisableHumanMatchID=") + std::to_string(BotId);
    }
    else
    {
        argument = std::string(" -F EnableHumanMatchID=") + std::to_string(BotId);

    }
    arguments.push_back(argument);
    std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
    std::string ErrorString;
    MainApp->RefreshBotsList(ListResult, ErrorString);
}

void BotsList::PrintWidth()
{
    QHeaderView* header = ui.BotsListTable->horizontalHeader();
    for (int ii = 0; ii < header->count(); ++ii) {
        qDebug() << "column" << ii << ":" << ui.BotsListTable->columnWidth(ii);
    }
    qDebug() << "tree widget width: " << ui.BotsListTable->width() << ", dialog width:" << width();
}

void BotsList::ToggleVsBot(const int BotId, bool Enable)
{
    vector<string> arguments;
    std::string argument = " -F Username=" + MainApp->ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + MainApp->ServerToken;
    arguments.push_back(argument);
    if (Enable)
    {
        argument = std::string(" -F DisableBotMatchID=") + std::to_string(BotId);
    }
    else
    {
        argument = std::string(" -F EnableBotMatchID=") + std::to_string(BotId);

    }
    arguments.push_back(argument);
    std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
    std::string ErrorString;
    MainApp->RefreshBotsList(ListResult, ErrorString);
}

void BotsList::ToggleDeactivated(const int BotId, bool Enable)
{
    vector<string> arguments;
    std::string argument = " -F Username=" + MainApp->ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + MainApp->ServerToken;
    arguments.push_back(argument);
    if (Enable)
    {
        argument = std::string(" -F EnableBotID=") + std::to_string(BotId);
    }
    else
    {
        argument = std::string(" -F DisableBotID=") + std::to_string(BotId);

    }
    arguments.push_back(argument);
    std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
    std::string ErrorString;
    MainApp->RefreshBotsList(ListResult, ErrorString);
}

void BotsList::ToggleDownloadable(const int BotId, bool Enable)
{
    vector<string> arguments;
    std::string argument = " -F Username=" + MainApp->ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + MainApp->ServerToken;
    arguments.push_back(argument);
    if (Enable)
    {
        argument = std::string(" -F DisableDownloadableID=") + std::to_string(BotId);
    }
    else
    {
        argument = std::string(" -F EnableDownloadableID=") + std::to_string(BotId);

    }
    arguments.push_back(argument);
    std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
    std::string ErrorString;
    MainApp->RefreshBotsList(ListResult, ErrorString);
}


void BotsList::SetColumnWidths()
{
    ui.BotsListTable->setColumnWidth(2, 75);
    ui.BotsListTable->setColumnWidth(3, 75);
    ui.BotsListTable->setColumnWidth(4, 75);
    ui.BotsListTable->setColumnWidth(5, 85);
    ui.BotsListTable->setColumnWidth(6, 75);
    ui.BotsListTable->setColumnWidth(7, 75);
    ui.BotsListTable->setColumnWidth(8, 75);

}

void BotsList::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    SetColumnWidths();
    OnRefreshButtonPressed();
}

void BotsList::UpdateBot(const std::string& BotName, std::string BotRace)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Bot"), "", tr("Zip Files (*.zip)"));
    if (fileName != "")
    {
        std::string Error;
        MainApp->UploadBot(BotName, BotRace, fileName.toStdString(), Error);
        MainApp->ShowError(Error);
    }
}
