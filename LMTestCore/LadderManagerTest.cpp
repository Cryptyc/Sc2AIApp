#include "LadderManagerTest.h"

#include <iostream>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_arg_parser.h"

#include "sc2utils/sc2_manage_process.h"

#include "Tools.h"
#include "AgentsConfig.h"
#include "LadderGame.h"

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <filesystem>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

#undef GetObject

#include "rapidjson.h"


#define PORT_START 5690
#define BOT_DOWNLOAD_PATH "https://sc2ai.net/DownloadPublicBot.php"
#define BOT_REQUEST_PATH "https://sc2ai.net/GetHumanPlayableBots.php"
#define UPLOAD_PATH "https://sc2ai.net/HumanUpload.php/"
#define TESTABLE_BOTS_PATH "https://sc2ai.net/GetTestableBots.php"

namespace fs = std::filesystem;
/*
std::string BaseConfigFile = "{ \n\
\"LocalReplayDirectory\" : \"Replays/\", \n\
\"EnableReplayUpload\" : \"False\", \n\
\"EnableServerLogin\" : \"False\", \n\
\"ResultsLogFile\" : \"Results.json\", \n\
\"PlayerIdFile\" : \"PlayerIds\", \n \
\"PythonBinary\" : \"python\", \n\
\"ReplayBotRenameProgram\" : \"Data\\BotReplayRename.exe\" \n\
}";
*/
std::string BaseConfigFile = "{ \n\
    \"LocalReplayDirectory\" : \"Replays/ \", \n\
        \"EnableReplayUpload\" : \"False\", \n\
        \"EnableServerLogin\" : \"False\", \n\
        \"ResultsLogFile\" : \"Results.json\", \n\
        \"PlayerIdFile\" : \"PlayerIds\", \n\
        \"PythonBinary\" : \"python\", \n\
        \"ReplayBotRenameProgram\" : \"Data/BotReplayRename.exe\" \n\
}";

std::string GetDocumentsFolder()
{
    TCHAR szPath[MAX_PATH];
    szPath[0] = '\0';
    std::string ReturnPath;
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath)))
    {
        PathAppend(szPath, TEXT("LadderManager/"));
        MakeDirectory(szPath);
        ReturnPath = szPath;
        PathAppend(szPath, TEXT("Replays/"));
        MakeDirectory(szPath);
    }
    return ReturnPath;
}


int mainX(int argc, char** argv)
{
    if (argc < 6)
    {
        std::cout << "Not enough commnad argument";
        return 0;
    }
    std::string BotName = argv[1];
    std::string MapName = argv[2];
    std::string TestBotName = argv[3];
    std::string RaceString = argv[4];
    std::string Username = argv[5];
    std::string Token = argv[6];
    std::string IsBotString = argv[7];
    std::string Region = argv[8];
    bool PlayVsHuman = false;
    if (IsBotString == "Human")
    {
        PlayVsHuman = true;
    }
    PrintThread{} << "Starting game " << TestBotName << " vs  " << BotName << " On " << MapName << std::endl;

    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();

    sc2::Race NewRace = GetRaceFromString(RaceString);

    NewGame->StartGame(BotName, TestBotName, MapName, Username, Token, PlayVsHuman, false, true,  NewRace);

    return 0;
}

BotConfig GetHumanBot(sc2::Race Race)
{
    BotConfig HumanBot;
    HumanBot.Type = BotType::Human;
    HumanBot.Race = Race;
    HumanBot.BotName = "Human";
    HumanBot.Skeleton = false;
    HumanBot.PlayerId = "HUMAN";
    return HumanBot;
}

BotConfig GetProxyBot(std::string BotName, std::string username, std::string password, std::string ProxyServer)
{
    BotConfig ProxyBot;
    ProxyBot.Type = BotType::RemoteTestBot;
    ProxyBot.Race = sc2::Race::Random;
    ProxyBot.BotName = BotName;
    ProxyBot.Skeleton = false;
    ProxyBot.PlayerId = BotName;
    ProxyBot.Username = username;
    ProxyBot.Password = password;
    ProxyBot.FileName = ProxyServer;
    return ProxyBot;
}


namespace LMHuman
{
    bool LMHumanGame::DownloadBot(const std::string& BotName, const std::string& Checksum, bool Data)
    {
        constexpr int DownloadRetrys = 3;
        std::string RootPath = BotsDirectory + "/" + BotName;
        if (Data)
        {
            RootPath += "/data";
        }
        RemoveDirectoryRecursive(RootPath);
        for (int RetryCount = 0; RetryCount < DownloadRetrys; RetryCount++)
        {
            std::vector<std::string> arguments;
            std::string argument = " -F BotName=" + BotName;
            arguments.push_back(argument);
            if (Data)
            {
                argument = " -F Data=1";
                arguments.push_back(argument);
            }
            std::string BotZipLocation = BotsDirectory + "/" + BotName + ".zip";
            remove(BotZipLocation.c_str());
            argument = " -o " + BotZipLocation;
            arguments.push_back(argument);
            PerformRestRequest(BOT_DOWNLOAD_PATH, arguments);
            std::string BotMd5 = GenerateMD5(BotZipLocation);
            PrintThread{} << "Download checksum: " << Checksum << " Bot checksum: " << BotMd5 << std::endl;

            if (BotMd5.compare(Checksum) == 0)
            {
                UnzipArchive(BotZipLocation, RootPath);
                remove(BotZipLocation.c_str());
                return true;
            }
            remove(BotZipLocation.c_str());
        }
        return false;
    }

    bool LMHumanGame::GetBot(const std::string& BotName, const std::string& BotChecksum, const std::string& DataChecksum)
    {
        if (!DownloadBot(BotName, BotChecksum, false))
        {
            std::string FailureMessage = "Unable to download bot " + BotName;
            LogFailiure(FailureMessage);
            return false;
        }

        if (DataChecksum != "")
        {
            if (!DownloadBot(BotName, DataChecksum, true))
            {
                std::string FailureMessage = "Unable to download data for " + BotName;
                LogFailiure(FailureMessage);
                return false;
            }
        }
        else
        {
            const std::string DataLocation = BotsDirectory + "/" + BotName + "/data";
            MakeDirectory(DataLocation);
        }
        const std::string BotLocation = BotsDirectory + "/" + BotName;
        AgentConfig->LoadAgents(BotLocation, BotLocation + "/ladderbots.json");

        return true;
    }

    void LMHumanGame::LogFailiure(std::string ErrorMessage)
    {
    }

    LMHumanGame::LMHumanGame()
    {
        //        BaseConfigDirectory =  GetDocumentsFolder();
        BaseConfigDirectory = fs::current_path().string();// "./"; // GetDocumentsFolder();
        BotsDirectory = BaseConfigDirectory + "/Bots/";
        CreateConfig();
    }

    void LMHumanGame::SetBotDirectory(const std::string& NewBotDirectory)
    {
        BotsDirectory = NewBotDirectory + '/'; 
    }

    BotConfig LMHumanGame::ParseBotDetails(const std::string& BotName, const std::string& ListDocument)
    {
        std::vector<std::string> arguments;
        rapidjson::Document doc;
        bool parsingFailed = doc.Parse(ListDocument.c_str()).HasParseError();
        if (parsingFailed)
        {
            PrintThread{} << "Unable to parse incoming upload result: " << ListDocument << std::endl;
        }
        BotConfig FoundBot;
        if (doc.IsArray())
        {
            for (const auto& ArrayValue : doc.GetArray())
            {
                if (ArrayValue.IsObject())
                {
                    const auto& BotObject = ArrayValue.GetObject();
                    if (BotObject.HasMember("BotName") && BotObject["BotName"].IsString() && BotObject.HasMember("Race") && BotObject["Race"].IsString()
                        && BotObject.HasMember("checksum") && BotObject["checksum"].IsString())
                    {
                        if (BotName.compare(BotObject["BotName"].GetString()) == 0)
                        {
                            FoundBot.Skeleton = true;
                            FoundBot.BotName = BotName;
                            FoundBot.Race = GetRaceFromString(BotObject["Race"].GetString());
                            FoundBot.CheckSum = BotObject["checksum"].GetString();
                            if (BotObject.HasMember("datachecksum") && BotObject["datachecksum"].IsString())
                            {
                                FoundBot.DataCheckSum = BotObject["datachecksum"].GetString();
                            }
                        }

                    }
                }
            }
        }
        return FoundBot;
    }

    BotConfig LMHumanGame::GetBotDetails(const std::string& Username, const std::string& Token, const std::string& BotName)
    {
        std::vector<std::string> arguments;
        std::string argument = " -F Username=" + Username;
        arguments.push_back(argument);
        argument = " -F Token=" + Token;
        arguments.push_back(argument);
        std::string ListResult = PerformRestRequest(TESTABLE_BOTS_PATH, arguments);
        return ParseBotDetails(BotName, ListResult);

    }

    GameResult LMHumanGame::StartGame(std::string TestBotName, std::string RemoteBotName, std::string MapName, std::string username, std::string Token, bool IsHuman, bool IsRemoteBot, bool IsRealtime, sc2::Race Race)
    {
        BotConfig Agent;
        GameResult result;
        BotConfig TestBot;
        Matchup ThisMatchup;
        AgentConfig->ReadBotDirectories(BotsDirectory);
        if (IsHuman)
        {
            TestBot = GetHumanBot(Race);
        }
        else
        {
            AgentConfig->FindBot(TestBotName, TestBot);
            if (TestBot.Skeleton)
            {
                PrintThread{} << "Unable to download bot " << TestBot.BotName << std::endl;
               
                return result;
            }
        }
        if (IsRemoteBot)
        {
            Agent = GetBotDetails(username, Token, RemoteBotName);
            if (!GetBot(RemoteBotName, Agent.CheckSum, Agent.DataCheckSum))
            {
                return result;
            }
            const std::string BotLocation = Config->GetStringValue("BaseBotDirectory") + "/" + Agent.BotName;
            AgentConfig->LoadAgents(BotLocation, BotLocation + "/ladderbots.json");
        }
        AgentConfig->FindBot(RemoteBotName, Agent);
        if (Agent.Skeleton)
        {
            PrintThread{} << "Unable to download bot " << Agent.BotName << std::endl;
            return result;
        }
        
        ThisMatchup.Agent2 = Agent;
        ThisMatchup.Agent1 = TestBot;
        ThisMatchup.Map = MapName;
        LadderGame* NewGame = new LadderGame(BaseConfigDirectory + "/Replays/", Config->GetStringValue("ReplayBotRenameProgram"), IsRealtime);

        result = NewGame->StartGame(TestBot, Agent, MapName);
        PrintThread{} << "Game finished with result: " << GetResultType(result.Result) << std::endl << std::endl;
        if (IsHuman)
        {
            UploadCmdLine(result, ThisMatchup, UPLOAD_PATH);
        }

        return result;
    }

    bool LMHumanGame::CreateConfig()
    {
        std::string ConfigFileLocation = BaseConfigDirectory + "/HumanLadder.json";
        if (!fs::exists(ConfigFileLocation))
        {
            std::ofstream ofs(ConfigFileLocation);
            ofs << BaseConfigFile;
            ofs.close();
        }
        Config = new LadderConfig(ConfigFileLocation);
        if (!Config->ParseConfig())
        {
            PrintThread{} << "Unable to parse config (not found or not valid):  " << ConfigFileLocation << std::endl;
            return false;
        }
        AgentConfig = new AgentsConfig(Config);
        return true;
    }

    bool LMHumanGame::UploadCmdLine(GameResult result, const Matchup& ThisMatch, const std::string UploadResultLocation)
    {
        std::string ReplayDir = BaseConfigDirectory + "/Replays/";
        std::string RawMapName = RemoveMapExtension(ThisMatch.Map);
        std::string ReplayFile;
        ReplayFile = ThisMatch.Agent1.BotName + "v" + ThisMatch.Agent2.BotName + "-" + RawMapName + ".Sc2Replay";
        ReplayFile.erase(remove_if(ReplayFile.begin(), ReplayFile.end(), isspace), ReplayFile.end());
        std::string ReplayLoc = ReplayDir + ReplayFile;

        std::vector<std::string> arguments;
        std::string  argument = " -b cookies.txt";
        arguments.push_back(argument);
        argument = " -F Bot1Name=" + ThisMatch.Agent1.BotName;
        arguments.push_back(argument);
        argument = " -F Bot1Race=" + std::to_string((int)ThisMatch.Agent1.Race);
        arguments.push_back(argument);
        argument = " -F Bot2Name=" + ThisMatch.Agent2.BotName;
        arguments.push_back(argument);
        argument = " -F Bot2Race=" + std::to_string((int)ThisMatch.Agent2.Race);
        arguments.push_back(argument);
        argument = " -F Bot1AvgFrame=" + std::to_string(result.Bot1AvgFrame);
        arguments.push_back(argument);
        argument = " -F Bot2AvgFrame=" + std::to_string(result.Bot2AvgFrame);
        arguments.push_back(argument);
        argument = " -F Frames=" + std::to_string(result.GameLoop);
        arguments.push_back(argument);
        argument = " -F Map=" + RawMapName;
        arguments.push_back(argument);
        argument = " -F Result=" + GetResultType(result.Result);
        arguments.push_back(argument);
        argument = " -F replayfile=@" + ReplayLoc;
        arguments.push_back(argument);
        PerformRestRequest(UploadResultLocation, arguments);
        return true;
    }

}

/*
bool StartGameVsAI(std::string BotName, std::string MapName, int Race, std::string BotChecksum, std::string DataChecksum)
{
    LMHuman::LMHumanGame* NewGame = new LMHuman::LMHumanGame();
    if (!NewGame->GetBot(BotName, BotChecksum, DataChecksum))
    {
        return false;
    }
    NewGame->StartGame(BotName, MapName, Race);
    return true;
}

*/
