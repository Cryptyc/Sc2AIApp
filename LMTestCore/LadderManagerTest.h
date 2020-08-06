#pragma once
#include <iostream>
#include <sc2api/sc2_api.h>
#include "Types.h"

class AgentsConfig;
class LadderConfig;

namespace LMHuman
{


    class LMHumanGame
    {
    private:
        bool CreateConfig();
        bool UploadCmdLine(GameResult result, const Matchup& ThisMatch, const std::string UploadResultLocation);
        AgentsConfig* AgentConfig;
        LadderConfig* Config;
        std::string BaseConfigDirectory = "";
        std::string MapName = "";
        std::string BotName = "";
        std::string BotsDirectory = "";

    public:
        LMHumanGame();
        void SetBotDirectory(const std::string& NewBotDirectory);
        GameResult StartGame(std::string TestBotName, std::string RemoteBotName, std::string MapName, std::string username, std::string password, bool IsHuman, bool IsRemoteBot, bool IsRealtime, sc2::Race Race = sc2::Race::Random);
        bool DownloadBot(const std::string& BotName, const std::string& Checksum, bool Data);
        bool GetBot(const std::string& BotName, const std::string& BotChecksum, const std::string& DataChecksum);
        void LogFailiure(std::string ErrorMessage);
        BotConfig GetBotDetails(const std::string& Username, const std::string& Token, const std::string& BotName);
        BotConfig ParseBotDetails(const std::string& BotName, const std::string& ListDocument);


    };
}
//bool StartGameVsAI(const char* BotName, const char* MapName, int Race);
