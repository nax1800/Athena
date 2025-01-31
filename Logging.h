#pragma once
#include "framework.h"
#include <fstream>
#include <string>
#include <mutex>

enum ELogType : uint8_t
{
    Invalid = 0,
    Abilities = 1,
    Actor = 2,
    Player = 3,
    Game = 4,
    Inventory = 5,
    Kismet = 6,
    Hook = 7,
    Athena = 8,
    ProcessEvent = 9,
    Quests = 10,
    Bot = 11
};

enum ELogEvent : uint8_t
{
    Warning = 1,
    Info = 2,
    Error = 3
};

/*
tbh idek what im doing here but itll be better later
*/

namespace Logging
{
    std::mutex logMutex;

    std::string LogTypeToString(ELogType LogType = ELogType::Invalid)
    {
        switch (LogType)
        {
        case ELogType::Abilities: return "LogAbilities";
        case ELogType::Actor: return "LogActor";
        case ELogType::Player: return "LogPlayer";
        case ELogType::Game: return "LogGame";
        case ELogType::Inventory: return "LogInventory";
        case ELogType::Kismet: return "LogKismet";
        case ELogType::Hook: return "LogHook";
        case ELogType::Athena: return "LogAthena";
        case ELogType::ProcessEvent: return "LogProcessEvent";
        case ELogType::Quests: return "LogQuests";
        case ELogType::Bot: return "LogAI";
        default: return "Log";
        }
    }

    std::string LogEventToString(ELogEvent LogEvent)
    {
        switch (LogEvent)
        {
        case ELogEvent::Warning: return "Warning";
        case ELogEvent::Info: return "Info";
        case ELogEvent::Error: return "Error";
        default: return "Unknown";
        }
    }

    void Log(ELogEvent LogEvent, ELogType LogType, const char* Format, ...)
    {
        std::string Prefix = LogTypeToString(LogType) + ": " + LogEventToString(LogEvent) + ": ";

        char Buffer[1024];
        va_list _ArgList;
        va_start(_ArgList, Format);
        vsnprintf(Buffer, sizeof(Buffer), Format, _ArgList);
        va_end(_ArgList);

        std::string FullMessage = Prefix + Buffer + "\n";

        fprintf(stdout, "%s", FullMessage.c_str());

        {
            std::lock_guard<std::mutex> lock(logMutex);
            std::ofstream logFile("Athena.log", std::ios::app);
            if (logFile.is_open())
            {
                logFile << FullMessage;
                logFile.close();
            }
        }
    }
}