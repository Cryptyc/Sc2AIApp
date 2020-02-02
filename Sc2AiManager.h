#pragma once

#include "Types.h"

class QApplication;
class LoginWindow;
class Sc2AiApp;
class ConfirmButton;
class BotsList;

#define BOTS_DIRECTORY "Bots"
#define BOT_UPLOAD_PATH "http://sc2ai.net/AppUpload.php"
#define LOGIN_TOKEN_PATH "http://sc2ai.net/CheckAppLogin.php"
#define REGIONS_PATH "http://sc2ai.net/GetRegions.php"
#define MAPS_PATH "http://sc2ai.net/GetCurrentMaps.php"
#define HUMAN_BOTS_PATH "http://sc2ai.net/GetTestableBots.php"
#define TESTABLE_BOTS_PATH "http://sc2ai.net/GetHumanTestableBots.php"
#define LIST_BOTS_PATH "https://sc2ai.net/ListUserBots.php"
#define DATA_DOWNLOAD_PATH "https://sc2ai.net/AppDataDownload.php"

struct BotData
{
    std::string BotName;
    sc2::Race Race;
    bool Verified;
    bool Deactivated;
    bool Deleted;
    bool HumanPlayable;
    bool BotPlayable;
    bool Rejected;
    bool Downloadable;
    int Id;
};

class Sc2AiManager
{

public:
    int RunApp();
    Sc2AiManager(QApplication* Inapp);

    void ShowError(std::string ErrorMessage);

    std::vector<std::string> GetRemoteBotList(std::string ListLocation);

    std::map<std::string, std::string> GetServerRegions();
    int ShowMainDoalog();
    void GetUserBots();
    std::string PrepareBotDirectory(const std::string& BotName, const std::string& BotRace, std::string& ErrorMessage);
    bool UploadBot(const std::string& BotName, const std::string& BotRace, const std::string& BotZipLocation, std::string& ErrorMessage);
    bool LoginRequest(std::string Username, std::string Password, std::string& ErrorResult);
    void RefreshLocalBots(std::string NewDir);
    bool RefreshBotsList(std::string ResultString, std::string& ErrorResult);
    std::vector<std::string> GetLocalBotDirs(std::string NewDir, bool NeedConfig);
    std::vector<std::string> GetActiveMaps();

    bool WriteBotConfig(std::string& ErrorMessage);

	std::string GenerateLadderConfig();

    void ToggleBotsListWindow(bool Show);

    std::string ServerUsername;
    std::string ServerToken;
    std::string CurrentBotDir;

private:

    bool VerifyUploadRequest(const std::string& UploadResult, std::string& ErrorMessage);

    QApplication* app;
    LoginWindow* AppLoginWindow;
    Sc2AiApp* AppSc2Ai;
    ConfirmButton* AppConfirmButton;
    BotsList* BotListWindow;
    std::map<std::string, std::string> Regions;
    std::map<std::string, BotData> ServerBots;
};
