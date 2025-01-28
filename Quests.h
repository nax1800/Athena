#pragma once
#include "framework.h"

namespace QuestsHandler
{
	void Initialize()
	{

		Logging::Log(ELogEvent::Info, ELogType::Hook, "Quests hooks initialized.");
	}
}