#pragma once
#include "framework.h"

namespace AI
{
	int BotsToSpawn = 6;
	// APlayerPawn_Athena_C
	APlayerPawn_Athena_C* SpawnBot(FVector Location)
	{
		auto Pawn = Spawner::SpawnActor<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass(), Location);
		if (!Pawn)
		{
			Logging::Log(ELogEvent::Warning, ELogType::Bot, "Failed to spawn bot.");
			return nullptr;
		}

		auto PlayerController = Spawner::SpawnActor<AAthena_PlayerController_C>(AAthena_PlayerController_C::StaticClass(), Location);
		auto PlayerState = Spawner::SpawnActor<AFortPlayerStateAthena>(AFortPlayerStateAthena::StaticClass(), Location);
		PlayerController->Player = reinterpret_cast<UPlayer*>(UGameplayStatics::SpawnObject(UPlayer::StaticClass(), PlayerController));
		PlayerController->PlayerState = PlayerState;
		PlayerController->OnRep_PlayerState();

		Pawn->PlayerState = PlayerState;
		Pawn->OnRep_PlayerState();
		Pawn->Controller = PlayerController;


		if (!Pawn->CharacterMovement)
		{
			Logging::Log(ELogEvent::Warning, ELogType::Bot, "Wild.");
			Pawn->CharacterMovement = reinterpret_cast<UCharacterMovementComponent*>(UGameplayStatics::SpawnObject(UCharacterMovementComponent::StaticClass(), Pawn));
		}

		static auto HeadPart = StaticFindObject<UCustomCharacterPart>(L"/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1");
		static auto BodyPart = StaticFindObject<UCustomCharacterPart>(L"/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01");

		PlayerState->CharacterParts.Parts[0] = HeadPart;
		PlayerState->CharacterParts.Parts[1] = BodyPart;
		PlayerState->OnRep_CharacterParts();

		PlayerState->bIsSpectator = false;
		PlayerState->bIsABot = true;

		PlayerController->Possess(Pawn);

		Pawn->PawnUniqueID = rand() % 1000;
		Pawn->OnRep_PawnUniqueID();

		static vector<FString> BotNames{
			L"SuperNax89", L"BlackSoy79", L"BlackHole9", L"GameMaster45", L"AthenaBot1", L"JimmyJim3"
		};

		if (BotNames.empty())
		{
			Logging::Log(ELogEvent::Error, ELogType::Bot, "Could not spawn a bot, ran out of names.");
			return nullptr;
		}

		int index = rand() % BotNames.size();
		FString BotName = BotNames[index];
		PlayerController->ServerChangeName(BotName);
		BotNames.erase(BotNames.begin() + index);

		PlayerState->bHasFinishedLoading = true;
		PlayerState->bHasStartedPlaying = true;
		PlayerState->OnRep_bHasStartedPlaying();
		PlayerController->ServerSetClientHasFinishedLoading(true);
		PlayerController->ServerLoadingScreenDropped();
		PlayerController->bHasInitiallySpawned = true;
		PlayerController->bAssignedStartSpawn = true;
		PlayerController->bReadyToStartMatch = true;
		PlayerController->bClientPawnIsLoaded = true;
		PlayerController->bHasClientFinishedLoading = true;
		PlayerController->bHasServerFinishedLoading = true;
		PlayerController->OnRep_bHasServerFinishedLoading();

		Pawn->SetMaxHealth(100.f);
		Pawn->SetHealth(100.f);

		auto CheatManager = reinterpret_cast<UFortCheatManager*>(UGameplayStatics::SpawnObject(UFortCheatManager::StaticClass(), PlayerController));
		PlayerController->CheatManager = CheatManager;
		CheatManager->ToggleInfiniteAmmo();

		static auto PickaxeWID = StaticFindObject<UFortWeaponMeleeItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		auto WorldItem = InventoryHandler::CreateItem(PlayerController, PickaxeWID);
		Pawn->EquipWeaponDefinition(PickaxeWID, WorldItem->GetItemGuid());

		PlayerController->ServerAcknowledgePossession(Pawn);

		EFortTeam Old = PlayerState->TeamIndex;
		PlayerState->TeamIndex = EFortTeam::HumanPvP_Team1;
		PlayerState->OnRep_TeamIndex(Old);

		PlayerController->ServerReadyToStartMatch();

		Globals::GetGameState()->PlayersLeft++;
		Globals::GetGameState()->OnRep_PlayersLeft();

		Globals::GetGameState()->PlayerArray.Add(PlayerState);
		Globals::GetGameMode()->AlivePlayers.Add(PlayerController);

		Logging::Log(ELogEvent::Info, ELogType::Bot, "Spawned bot: %s", BotName.ToString().c_str());

		return Pawn;
	}
}