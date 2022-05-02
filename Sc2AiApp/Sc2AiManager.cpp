#include "Sc2AiManager.h"
#include <QtWidgets/QApplication>
#include <qmessagebox.h>
#include <qobject.h>
#include "Sc2AiApp.h"
#include "LoginWindow.h"
#include "ConfirmButton.h"
#include "BotsList.h"
#include "Tools.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <filesystem>

using namespace std;
using namespace rapidjson;
namespace fs = std::filesystem;


string EscapeString(const string InString)
{
    string OutString;
    for (char c : InString)
    {
        // check if a given character is printable
        // the cast is necessary to avoid undefined behaviour
        if (isprint((unsigned char)c))
        {
            OutString += c;
        }
        else
        {
            std::stringstream stream;
            // if the character is not printable
            // we'll convert it to a hex string using a stringstream
            // note that since char is signed we have to cast it to unsigned first
            stream << std::hex << (unsigned int)(unsigned char)(c);
            std::string code = stream.str();
            OutString += std::string("\\x") + (code.size() < 2 ? "0" : "") + code;
            // alternatively for URL encodings:
            //s += std::string("%")+(code.size()<2?"0":"")+code;
        }
    }
    return OutString;
}


Sc2AiManager::Sc2AiManager(QApplication* Inapp)
    : app(Inapp)
{
    CurrentBotDir = BOTS_DIRECTORY;
}

void Sc2AiManager::ShowError(std::string ErrorMessage)
{

    QMessageBox::information(AppSc2Ai, QString::fromUtf8("Sc2AiApp"), QString::fromUtf8(ErrorMessage.c_str()));
}

std::vector<std::string> Sc2AiManager::GetRemoteBotList(std::string ListLocation)
{
    std::vector<std::string> FoundBots;

    std::vector<std::string> arguments;
    std::string argument = " -F Username=" + ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + ServerToken;
    arguments.push_back(argument);

    std::string BotsString = PerformRestRequest(ListLocation, arguments);
    Document doc;
    bool parsingFailed = doc.Parse(BotsString.c_str()).HasParseError();
    if (parsingFailed)
    {
        PrintThread{} << "Unable to parse incoming upload result: " << BotsString << std::endl;
    }
    if (doc.IsArray())
    {
        for (const auto& ArrayValue : doc.GetArray())
        {
            if (ArrayValue.IsObject())
            {
                const auto& BotObject = ArrayValue.GetObject();
                if (BotObject.HasMember("BotName") && BotObject["BotName"].IsString() && BotObject.HasMember("Race") && BotObject["Race"].IsString())
                {
                    FoundBots.push_back(BotObject["BotName"].GetString());
                }
            }
        }
    }
    return FoundBots;
}

int Sc2AiManager::ShowMainDoalog()
{
    AppLoginWindow->hide();
    AppSc2Ai->show();
    ToggleBotsListWindow(true);
    return 0;
}
void Sc2AiManager::GetUserBots()
{
    vector<string> arguments;
    std::string argument = " -F Username=" + ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + ServerToken;
    arguments.push_back(argument);
    std::string ListResult = PerformRestRequest(LIST_BOTS_PATH, arguments);
    std::string ErrorString;
    RefreshBotsList(ListResult, ErrorString);
}

std::string Sc2AiManager::PrepareBotDirectory(const std::string& BotName, const std::string& BotRace, std::string& ErrorMessage)
{
    std::string BotZipLocation = CurrentBotDir + "/" + BotName + ".zip";
    std::string InputLocation = CurrentBotDir + "/" + BotName;
    ZipArchive(InputLocation, BotZipLocation);
    return BotZipLocation;
}

bool Sc2AiManager::UploadBot(const std::string& BotName, const std::string& BotRace, const std::string& BotZipLocation, const bool IsNewBot, std::string& ErrorMessage)
{
    string EscapedBotLocation = string("\"") + BotZipLocation + string("\"");
    string EscapedBotName = string("\"") + BotName + string("\"");

    std::string BotMd5 = GenerateMD5(BotZipLocation);
    std::vector<std::string> arguments;
    std::string argument = " -F Username=" + ServerUsername;
    arguments.push_back(argument);
    argument = " -F Token=" + ServerToken;
    arguments.push_back(argument);
    argument = " -F BotName=" + BotName;
    arguments.push_back(argument);
    if (AppSc2Ai->GetDowloadable() && IsNewBot)
    {
        argument = " -F Downloadable=1";
        arguments.push_back(argument);
    }
    if (AppSc2Ai->GetHumanPlayable() && IsNewBot)
    {
        argument = " -F HumanPlayable=1";
        arguments.push_back(argument);
    }
    argument = " -F BotPlayable=1";
    arguments.push_back(argument);
    argument = " -F Race=" + BotRace;
    arguments.push_back(argument);
    argument = " -F Checksum=" + BotMd5;
    arguments.push_back(argument);
    argument = " -F BotFile=@" + BotZipLocation;
    arguments.push_back(argument);

    constexpr int UploadRetrys = 3;
    for (int RetryCount = 0; RetryCount < UploadRetrys; RetryCount++)
    {
        std::string UploadResult = PerformRestRequest(BOT_UPLOAD_PATH, arguments);
        if (VerifyUploadRequest(UploadResult, ErrorMessage))
        {
            SleepFor(1);
            if (IsNewBot)
            {
                remove(BotZipLocation.c_str());
            }
            GetUserBots();
            return true;
        }
    }
    if (IsNewBot)
    {
        remove(BotZipLocation.c_str());
    }
    return false;
}

bool Sc2AiManager::VerifyUploadRequest(const std::string& UploadResult, std::string& ErrorMessage)
{
    Document doc;
    bool parsingFailed = doc.Parse(UploadResult.c_str()).HasParseError();
    if (parsingFailed)
    {
        PrintThread{} << "Unable to parse incoming upload result: " << UploadResult << std::endl;
        ErrorMessage = "Unable to parse upload result";
        return false;
    }
    if (doc.HasMember("result") && doc["result"].IsBool() && doc["result"].GetBool())
    {
        ErrorMessage = "Upload Successful";
        return true;
    }
    if (doc.HasMember("error") && doc["error"].IsBool())
    {
        PrintThread{} << "Error uploading bot: " << doc["error"].GetString() << std::endl;
        ErrorMessage = doc["error"].GetString();
        return false;
    }
    ErrorMessage = "Unknown Error";
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

    Document doc;
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

void Sc2AiManager::RefreshLocalBots(std::string NewDir)
{
    CurrentBotDir = NewDir;
}

bool Sc2AiManager::RefreshBotsList(std::string ResultString, std::string& ErrorResult)
{

    Document doc;
    bool parsingFailed = doc.Parse(ResultString.c_str()).HasParseError();
    if (parsingFailed)
    {
        ErrorResult = "Request Error";
        return false;
    }
    if (doc.HasMember("result") && doc["result"].IsBool())
    {
        if (!doc["result"].GetBool())
        {
            ErrorResult = "Unable to get bots list";
            return false;
        }
        if (!(doc.HasMember("Bots") && doc["Bots"].IsArray()))
        {
            ErrorResult = "Not Bots in result";
            return true;
        }
        ServerBots.clear();
        for (const auto& ArrayValue : doc["Bots"].GetArray())
        {
            if (ArrayValue.IsObject())
            {
                const auto& BotsObject = ArrayValue.GetObject();
                BotData NewBotData;
                if (BotsObject.HasMember("Name") && BotsObject["Name"].IsString())
                {
                    NewBotData.BotName = BotsObject["Name"].GetString();
                    if (BotsObject.HasMember("Race") && BotsObject["Race"].IsString())
                    {
                        NewBotData.Race = (sc2::Race)std::stoi(BotsObject["Race"].GetString());
                    }
                    if (BotsObject.HasMember("Verified") && BotsObject["Verified"].IsString())
                    {
                        NewBotData.Verified = std::stoi(BotsObject["Verified"].GetString()) == 1;
                    }
                    if (BotsObject.HasMember("Deactivated") && BotsObject["Deactivated"].IsString())
                    {
                        NewBotData.Deactivated = std::stoi(BotsObject["Deactivated"].GetString()) == 1;
                    }
                    if (BotsObject.HasMember("Deleted") && BotsObject["Deleted"].IsString())
                    {
                        NewBotData.Deleted = std::stoi(BotsObject["Deleted"].GetString()) == 1;
                    }
                    if (BotsObject.HasMember("HumanPlayable") && BotsObject["HumanPlayable"].IsString())
                    {
                        NewBotData.HumanPlayable = std::stoi(BotsObject["HumanPlayable"].GetString()) == 1;
                    }
                    if (BotsObject.HasMember("BotPlayable") && BotsObject["BotPlayable"].IsString())
                    {
                        NewBotData.BotPlayable = std::stoi(BotsObject["BotPlayable"].GetString()) == 1;
                    }
                    if (BotsObject.HasMember("Rejected") && BotsObject["Rejected"].IsString())
                    {
                        NewBotData.Rejected = std::stoi(BotsObject["Rejected"].GetString()) == 1;
                    }
                    if (BotsObject.HasMember("ID") && BotsObject["ID"].IsString())
                    {
                        NewBotData.Id = std::stoi(BotsObject["ID"].GetString());
                    }
                    if (BotsObject.HasMember("Downloadable") && BotsObject["Downloadable"].IsString())
                    {
                        NewBotData.Downloadable = std::stoi(BotsObject["Downloadable"].GetString()) == 1;
                    }
                }
                if (!NewBotData.Deleted)
                {
                    ServerBots[NewBotData.BotName] = NewBotData;
                }
            }
        }
        BotListWindow->RefreshList(ServerBots);

    }
    else
    {
        ErrorResult = "Request Error";
    }
    return true;
}

int Sc2AiManager::RunApp()
{
    AppLoginWindow = new LoginWindow(this);
    AppSc2Ai = new Sc2AiApp(this);
    AppConfirmButton = new ConfirmButton(this);
    BotListWindow = new BotsList(this);
    QRect CurrentGeometry = BotListWindow->geometry();
    CurrentGeometry.setY(CurrentGeometry.y() + AppSc2Ai->geometry().y());
    BotListWindow->setGeometry(CurrentGeometry);
    AppLoginWindow->hide();
    AppSc2Ai->show();
    BotListWindow->hide();
    return app->exec();
}

void Sc2AiManager::GetLocalBotDirs(std::string NewDir, bool NeedConfig, std::vector<std::string>& directories)
{
    if (NewDir != "")
    {
        CurrentBotDir = NewDir;
    }
    if (!filesystem::exists(CurrentBotDir))
    {
        return;
    }
    std::filesystem::directory_iterator dirIter(CurrentBotDir);
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
    return;
}

void Sc2AiManager::GetActiveMaps(std::vector<std::string>& ActiveMaps)
{
    ActiveMaps.clear();
    std::string ConfigFileLocation = fs::current_path().string() + "/HumanLadder.json";
    if (!fs::exists(ConfigFileLocation))
    {
        return;
    }
    std::ifstream t(ConfigFileLocation);
    if (!t.good())
    {
        return;
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string ConfigString = buffer.str();
    Document doc;
    if(doc.Parse(ConfigString.c_str()).HasParseError())
    {
        return;
    }
    if (doc.HasMember("Maps") && doc["Maps"].IsArray())
    {
        for (const auto& ArrayValue : doc["Maps"].GetArray())
        {
            if (ArrayValue.IsString())
            {
                ActiveMaps.push_back(ArrayValue.GetString());
            }
        }

    }
}

bool Sc2AiManager::WriteBotConfig(std::string& ErrorMessage)
{
    string ConfigFileLocation = string(CurrentBotDir) + string("/") + AppSc2Ai->GetBotName();
    if (!filesystem::exists(ConfigFileLocation))
    {
        ErrorMessage = "Bot directory does not exist.";
        return false;
    }
    string ExeLocation = ConfigFileLocation + string("/") + AppSc2Ai->GetFileName();
    if (!filesystem::exists(ExeLocation))
    {
        ErrorMessage = "Bot executable does not exist.";
        return false;
    }

    string ConfigString = GenerateLadderConfig();
    ConfigFileLocation += string("/ladderbots.json");
    std::ofstream ofs(ConfigFileLocation);
    ofs << ConfigString;
    ofs.close();
    ErrorMessage = "Ladder config has been written successfully";
    return true;
}

string Sc2AiManager::GenerateLadderConfig()
{
    Document ConfigDoc;
    ConfigDoc.SetObject();
    Document::AllocatorType& allocator = ConfigDoc.GetAllocator();
    Value BotsTopLevel(kObjectType);
    Value BotObject(kObjectType);

    BotObject.AddMember(Value("Race", allocator).Move(), Value(AppSc2Ai->GetRace(), allocator).Move(), allocator);
    BotObject.AddMember(Value("Type", allocator).Move(), Value(AppSc2Ai->GetAPI(), allocator).Move(), allocator);
    BotObject.AddMember(Value("RootPath", allocator).Move(), Value("./", allocator).Move(), allocator);
    BotObject.AddMember(Value("FileName", allocator).Move(), Value(AppSc2Ai->GetFileName(), allocator).Move(), allocator);
    if (AppSc2Ai->GetDebug())
    {
        BotObject.AddMember(Value("Debug", allocator).Move(), Value(AppSc2Ai->GetDebug()), allocator);
    }
    string Arguments = AppSc2Ai->GetArgs();
    if (Arguments != "")
    {
        BotObject.AddMember(Value("Args", allocator).Move(), Value(Arguments, allocator).Move(), allocator);
    }

    BotsTopLevel.AddMember(Value(AppSc2Ai->GetBotName(), allocator), BotObject, allocator);
    ConfigDoc.AddMember(Value("Bots", allocator), BotsTopLevel, allocator);
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    ConfigDoc.Accept(writer);
    return buffer.GetString();
}

void Sc2AiManager::ToggleBotsListWindow(bool Show)
{
    if (Show)
    {
        if (ServerToken.length() > 0 && ServerUsername.length() > 0)
        {
            BotListWindow->show();
        }
    }
    else
    {
        BotListWindow->hide();
    }
}
