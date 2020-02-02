#pragma once

#include "Types.h"

class QApplication;
class LoginWindow;
class Sc2AiApp;

#define BOTS_DIRECTORY "Bots"
#define BOT_UPLOAD_PATH "http://sc2ai.net/UploadBot.php"
#define LOGIN_TOKEN_PATH "http://sc2ai.net/CheckAppLogin.php"
#define REGIONS_PATH "http://sc2ai.net/GetRegions.php"
#define MAPS_PATH "http://sc2ai.net/GetCurrentMaps.php"

class Sc2AiManager
{

public:
    int RunApp();
    Sc2AiManager(QApplication* Inapp);

    std::map<std::string, std::string> GetServerRegions();
    int ShowMainDoalog();
    bool UploadBot(const BotConfig& bot, bool Data);
    bool LoginRequest(std::string Username, std::string Password, std::string& ErrorResult);
    std::vector<std::string> GetLocalBotDirs(bool NeedConfig);
    std::vector<std::string> GetActiveMaps();



private:

    bool VerifyUploadRequest(const std::string& UploadResult);

    QApplication* app;
    LoginWindow* AppLoginWindow;
    Sc2AiApp* AppSc2Ai;
    std::string ServerUsername;
    std::string ServerToken;
    std::map<std::string, std::string> Regions;
};
