#pragma once
#include "framework.h"

namespace GameMode
{
	bool (*oReadyToStartMatch)(void*);
	bool hkReadyToStartMatch(AFortGameModeAthena* GameMode)
	{
		auto GameState = reinterpret_cast<AFortGameStateAthena*>(GameMode->GameState);

		if (!Globals::bIsPlaylistSetup)
		{
			Globals::bIsPlaylistSetup = true;

			auto Playlist = StaticFindObject<UFortPlaylistAthena>(L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
			if (Playlist)
			{
				Logging::Log(ELogEvent::Info, ELogType::Athena, "Playlist: %s", Playlist->UIDisplayName.ToString().c_str());

				GameState->CurrentPlaylistId = Playlist->PlaylistId;
				FPlaylistPropertyArray& PlaylistInfo = GameState->CurrentPlaylistInfo;
				PlaylistInfo.BasePlaylist = Playlist;
				PlaylistInfo.OverridePlaylist = Playlist;
				PlaylistInfo.PlaylistReplicationKey++;
				PlaylistInfo.MarkArrayDirty();

				GameMode->CurrentPlaylistName = Playlist->PlaylistName;
				GameMode->CurrentPlaylistId = Playlist->PlaylistId;

				GameState->OnRep_CurrentPlaylistId();
				GameState->OnRep_CurrentPlaylistInfo();

				GameState->FriendlyFireType = Playlist->FriendlyFireType;

				GameMode->FortGameSession->MaxPlayers = Playlist->MaxPlayers;
				GameMode->FortGameSession->MaxPartySize = Playlist->MaxSocialPartySize;

				if (!Globals::bUseBeacons) // No idea why it does this tbh
				{
					Logging::Log(ELogEvent::Info, ELogType::Game, "GamePhase set to 'Setup'");
					EAthenaGamePhase OldGamePhase = GameState->GamePhase;
					GameState->GamePhase = EAthenaGamePhase::Setup;
					GameState->OnRep_GamePhase(OldGamePhase);
				}
			}
			else
				Logging::Log(ELogEvent::Error, ELogType::Athena, "Playlist is null.");
		}

		if (!GameState->MapInfo)
			return false;


		GameMode->DefaultPawnClass = SDK::APlayerPawn_Athena_C::StaticClass();

		if (!Globals::bIsServerListening)
		{
			GameMode->WarmupRequiredPlayerCount = 1;
			Server::Listen();
			Server::Initialize();
		}
		GameMode->bWorldIsReady = true;

		return oReadyToStartMatch(GameMode);
	}

	APawn* (*oSpawnDefaultPawnFor)(void* GameMode, AController* NewPlayer, AActor* StartSpot);
	APawn* hkSpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AController* NewPlayer, AActor* StartSpot)
	{
		if (!NewPlayer || !StartSpot)
			return nullptr;

		FTransform Transform = StartSpot->GetTransform();
		return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, Transform);
	}

	int hkPickTeam(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller)
	{
		return 3;
	}

	/*
		Deciding whether to do proper game sessions or nah.
	*/

	void Initialize()
	{
		MH_STATUS StatusReadyToStartMatch = Memory::CreateHook(Memory::GetAddress(0xcb45a0), hkReadyToStartMatch, (void**)&oReadyToStartMatch);
		MH_STATUS StatusSpawnDefaultPawnFor = Memory::CreateHook(Memory::GetAddress(0xcbb040), hkSpawnDefaultPawnFor);
		MH_STATUS StatusPickTeam = Memory::CreateHook(Memory::GetAddress(0xcb0890), hkPickTeam);

#ifdef LOG_HOOKSTATUS
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkReadyToStartMatch Status: %s.", MH_StatusToString(StatusReadyToStartMatch));
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkSpawnDefaultPawnFor Status: %s.", MH_StatusToString(StatusSpawnDefaultPawnFor));
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkPickTeam Status: %s.", MH_StatusToString(StatusPickTeam));
#endif // LOG_HOOKSTATUS


		Logging::Log(ELogEvent::Info, ELogType::Hook, "GameMode hooks initialized.");
	}
}