#pragma once
#include "framework.h"

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
	ProcessEvent = 9
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
	string LogTypeToString(ELogType LogType = ELogType::Invalid)
	{
		if (LogType == ELogType::Abilities)
			return "LogAbilities";
		else if (LogType == ELogType::Actor)
			return "LogActor";
		else if (LogType == ELogType::Player)
			return "LogPlayer";
		else if (LogType == ELogType::Game)
			return "LogGame";
		else if (LogType == ELogType::Inventory)
			return "LogInventory";
		else if (LogType == ELogType::Kismet)
			return "LogKismet";
		else if (LogType == ELogType::Hook)
			return "LogHook";
		else if (LogType == ELogType::Athena)
			return "LogAthena";
		else if (LogType == ELogType::ProcessEvent)
			return "LogProcessEvent";

		return "Unknown";
	}

	string LogEventToString(ELogEvent LogEvent)
	{
		if (LogEvent == ELogEvent::Warning)
			return "Warning";
		else if (LogEvent == ELogEvent::Info)
			return "Info";
		else if (LogEvent == ELogEvent::Error)
			return "Error";

		return "Unknown";
	}

	void Log(ELogEvent LogEvent, ELogType LogType, const char* Format, ...)
	{
		std::string Prefix = "" + LogTypeToString(LogType) + ":" + LogEventToString(LogEvent) + ": ";
		va_list _ArgList;
		va_start(_ArgList, Format);
		std::string FullFormat = Prefix + Format + "\n";
		vfprintf(stdout, FullFormat.c_str(), _ArgList);
		va_end(_ArgList);
	}
}