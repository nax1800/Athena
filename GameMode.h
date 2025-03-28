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
			auto Playlist = GameMode->PlaylistManager->AthenaPlaylists[1];
			if (Playlist)
			{
				for (int i = 0; i < GameMode->PlaylistManager->AthenaPlaylists.Num(); i++)
				{
					auto yh = GameMode->PlaylistManager->AthenaPlaylists[i];
					Logging::Log(ELogEvent::Info, ELogType::Athena, "AthenaPlaylists: Index %i - %s", i, yh->GetFullName().c_str());
				}

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
			ActorHandler::Initialize();
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
		static int CurrentIndex = 3;
		static int MaxTeamSize = Globals::GetGameState()->CurrentPlaylistInfo.OverridePlaylist->MaxTeamSize;
		static int CurrentSize = 0;

		if (CurrentSize >= MaxTeamSize)
			CurrentIndex++;

		CurrentSize++;

		return CurrentIndex;
	}

	void Initialize()
	{
		MH_STATUS StatusReadyToStartMatch = Memory::CreateHook(Memory::GetAddress(0x25bac60), hkReadyToStartMatch, (void**)&oReadyToStartMatch);
		MH_STATUS StatusSpawnDefaultPawnFor = Memory::CreateHook(Memory::GetAddress(0xa083a0), hkSpawnDefaultPawnFor);
		MH_STATUS StatusPickTeam = Memory::CreateHook(Memory::GetAddress(0x9fe680), hkPickTeam);

#ifdef LOG_HOOKSTATUS
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkReadyToStartMatch Status: %s.", MH_StatusToString(StatusReadyToStartMatch));
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkSpawnDefaultPawnFor Status: %s.", MH_StatusToString(StatusSpawnDefaultPawnFor));
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkPickTeam Status: %s.", MH_StatusToString(StatusPickTeam));
#endif // LOG_HOOKSTATUS


		Logging::Log(ELogEvent::Info, ELogType::Hook, "GameMode hooks initialized.");
	}
}