#pragma once
#include "framework.h"

namespace Globals
{
	bool bUseBeacons = false;
	bool bUseAccountID = false; // backend communication, etc...
	bool bNoMCP = true;

	bool bIsServerListening = false;
	bool bIsPlaylistSetup = false;

	int Port = 7777;

	UFortEngine* GetEngine()
	{
		static auto Engine = UObject::FindObject<UFortEngine>("FortEngine Transient.FortEngine_2147482594");
		return Engine;
	}

	UWorld* GetWorld()
	{
		return GetEngine()->GameViewport->World;
	}

	AFortGameModeAthena* GetGameMode()
	{
		return static_cast<AFortGameModeAthena*>(GetWorld()->AuthorityGameMode);
	}

	AFortGameStateAthena* GetGameState()
	{
		return static_cast<AFortGameStateAthena*>(GetWorld()->GameState);
	}
}