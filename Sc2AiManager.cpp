#include "Sc2AiManager.h"
#include <QtWidgets/QApplication>
#include "Sc2AiApp.h"
#include "LoginWindow.h"
#include "Tools.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <filesystem>

Sc2AiManager::Sc2AiManager(QApplication* Inapp)
    : app(Inapp)
{

}

std::map<std::string, std::string> Sc2AiManager::GetServerRegions()
{
    std::vector<std::string> arguments;
    std::string RegionsString = PerformRestRequest(REGIONS_PATH, arguments);
    rapidjson::Document doc;
    bool parsingFailed = doc.Parse(RegionsString.c_str()).HasParseError();
    if (parsingFailed)
    {
        PrintThread{} << "Unable to parse incoming upload result: " << RegionsString << std::endl;
    }
    if (doc.IsArray())
    {
        for (const auto& ArrayValue : doc.GetArray())
        {
            if (ArrayValue.IsObject())
            {
                const auto& RegionObject = ArrayValue.GetObject();
                if (RegionObject.HasMember("Name") && RegionObject["Name"].IsString() && RegionObject.HasMember("IP") && RegionObject["IP"].IsString())
                {
                    Regions[RegionObject["Name"].GetString()] = RegionObject["IP"].GetString();
                }
            }
        }
    }
    return Regions;
}

int Sc2AiManager::ShowMainDoalog()
{
    AppLoginWindow->hide();
    AppSc2Ai->show();
    return 0;
}

bool Sc2AiManager::UploadBot(const BotConfig& bot, bool Data)
{
    std::string BotZipLocation = std::string(BOTS_DIRECTORY) + "/" + bot.BotName + ".zip";
    std::string InputLocation = bot.RootPath;
    if (Data)
    {
        InputLocation += "/data";
    }
    ZipArchive(InputLocation, BotZipLocation);
    std::string BotMd5 = GenerateMD5(BotZipLocation);
    std::vector<std::string> arguments;
    std::string argument = " -F Username=" + ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + ServerToken;
    arguments.push_back(argument);
    argument = " -F BotName=" + bot.BotName;
    arguments.push_back(argument);
    argument = " -F Checksum=" + BotMd5;
    arguments.push_back(argument);
    if (Data)
    {
        argument = " -F Data=1";
        arguments.push_back(argument);
    }
    argument = " -F BotFile=@" + BotZipLocation;
    arguments.push_back(argument);
    constexpr int UploadRetrys = 3;
    for (int RetryCount = 0; RetryCount < UploadRetrys; RetryCount++)
    {
        std::string UploadResult = PerformRestRequest(BOT_UPLOAD_PATH, arguments);
        if (VerifyUploadRequest(UploadResult))
        {
            SleepFor(1);
            RemoveDirectoryRecursive(InputLocation);
            remove(BotZipLocation.c_str());
            return true;
        }
    }
    RemoveDirectoryRecursive(InputLocation);
    remove(BotZipLocation.c_str());
    return false;
}

bool Sc2AiManager::VerifyUploadRequest(const std::string& UploadResult)
{
    rapidjson::Document doc;
    bool parsingFailed = doc.Parse(UploadResult.c_str()).HasParseError();
    if (parsingFailed)
    {
        PrintThread{} << "Unable to parse incoming upload result: " << UploadResult << std::endl;
        return false;
    }
    if (doc.HasMember("result") && doc["result"].IsBool() && doc["result"].GetBool())
    {
        return true;
    }
    if (doc.HasMember("error") && doc["error"].IsBool())
    {
        PrintThread{} << "Error uploading bot: " << doc["error"].GetString() << std::endl;
    }
    return false;
}

bool Sc2AiManager::LoginRequest(std::string Username, std::string Password, std::string& ErrorResult)
{
    std::vector<std::string> arguments;
    std::string argument = " -F Username=" + Username;
    arguments.push_back(argument);
    argument = " -F Password=" + Password;
    arguments.push_back(argument);
    std::string UploadResult = PerformRestRequest(LOGIN_TOKEN_PATH, arguments);

    rapidjson::Document doc;
    bool parsingFailed = doc.Parse(UploadResult.c_str()).HasParseError();
    if (parsingFailed)
    {
        ErrorResult = "Login Error";
        return false;
    }
    if (doc.HasMember("verified") && doc["verified"].IsBool() && doc["verified"].GetBool())
    {
        if ( doc.HasMember("Token") && doc["Token"].IsString())
        {
            ServerUsername = Username;
            ServerToken = doc["Token"].GetString();
            return true;
        }
    }
    if (doc.HasMember("error") && doc["error"].IsBool())
    {
        ErrorResult = doc["error"].GetString();
    }
    else
    {
        ErrorResult = "Login Error";
    }
    return false;
}

int Sc2AiManager::RunApp()
{
    AppLoginWindow = new LoginWindow(this);
    AppSc2Ai = new Sc2AiApp(this);
    AppLoginWindow->show();
    return app->exec();
}

std::vector<std::string> Sc2AiManager::GetLocalBotDirs(bool NeedConfig)
{
    std::vector<std::string> directories;
    std::filesystem::directory_iterator dirIter(BOTS_DIRECTORY);
    for (const std::filesystem::directory_entry& Directory : dirIter)
    {
        if (Directory.is_directory())
        {
            if (NeedConfig)
            {
                if (std::filesystem::exists(Directory.path().string() + "/ladderbots.json"))
                {
                    directories.push_back(Directory.path().filename().string());
                }
            }
            else
            {
                directories.push_back(Directory.path().filename().string());
            }
        }
    }
    return directories;
}

std::vector<std::string> Sc2AiManager::GetActiveMaps()
{
    std::vector<std::string> ActiveMaps;
    std::vector<std::string> arguments;
    std::string RegionsString = PerformRestRequest(MAPS_PATH, arguments);
    rapidjson::Document doc;
    bool parsingFailed = doc.Parse(RegionsString.c_str()).HasParseError();
    if (parsingFailed)
    {
        PrintThread{} << "Unable to parse incoming upload result: " << RegionsString << std::endl;
    }
    if (doc.IsArray())
    {
        for (const auto& ArrayValue : doc.GetArray())
        {
            if (ArrayValue.IsString())
            {
                ActiveMaps.push_back(ArrayValue.GetString());
            }
        }
    }
    return ActiveMaps;
}

