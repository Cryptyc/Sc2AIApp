#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BotsList.h"
#include "Sc2AiManager.h"

class Sc2AiManager;

class BotsList : public QMainWindow
{
    Q_OBJECT

public:
    BotsList(Sc2AiManager* MainApp, QWidget* parent = Q_NULLPTR);
    void RefreshList(std::map<std::string, BotData> ServerBots);

private:
    Ui::BotsListWindow ui;
    Sc2AiManager* MainApp;

    std::string GetBotStatus(const BotData& Bot);

private:
    void DownloadData(const int BotId, const std::string& BotName);
    void DeleteBot(const int RowIndex);
    void UpdateBot(const std::string& BotName, std::string BotRace);
    void ToggleVsBot(const int RowIndex, bool Enable);
    void ToggleDeactivated(const int RowIndex, bool Enable);
    void ToggleDownloadable(const int BotId, bool Enable);
    void SetColumnWidths();
	virtual void showEvent(QShowEvent* event) override;
    void ToggleVsHuman(const int RowIndex, bool Enable);

    void PrintWidth();

private slots:
    void OnRefreshButtonPressed();

};
