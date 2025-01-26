#pragma once
#include "framework.h"

namespace Globals
{
	bool bUseBeacons = false;
	bool bUseAccountID = false; // backend communication, etc...

	bool bIsServerListening = false;
	bool bIsPlaylistSetup = false;

	int Port = 7777;


	AFortGameModeAthena* GetGameMode()
	{
		return static_cast<AFortGameModeAthena*>(UWorld::GetWorld()->AuthorityGameMode);
	}

	AFortGameStateAthena* GetGameState()
	{
		return static_cast<AFortGameStateAthena*>(UWorld::GetWorld()->GameState);
	}
}