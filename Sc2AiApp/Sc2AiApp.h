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
    std::string GetRace() { return ui.RaceCombo->currentText().toStdString(); }
    std::string GetAPI() { return ui.APICombo->currentText().toStdString(); }
    std::string GetFileName() { return ui.DirectoryEdit->text().toStdString(); }
    std::string GetBotName() { return ui.BotNameEdit->text().toStdString(); }
    std::string GetArgs() { return ui.ArgsEdit->text().toStdString(); }
    bool GetDebug() { return ui.DebugCheckBox->isChecked(); }
    bool GetDowloadable() { return ui.DownloadCheckBox->isChecked(); }
    bool GetHumanPlayable() { return ui.VsHumanCheckBox->isChecked(); }

private:
    Ui::Sc2AiAppDialog ui;
    Sc2AiManager* MainApp;

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
