#pragma once
#include "framework.h"

namespace PlayerController
{
	static void* (*ApplyCharacterCustomization)(void* a1, void* a2) = decltype(ApplyCharacterCustomization)(Memory::GetAddress(0x1348d00));
	void hkServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* Pawn)
	{
		if (!PlayerController)
			return;

		auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		auto FortPawn = static_cast<APlayerPawn_Athena_C*>(Pawn);

		if (!PlayerState || !FortPawn)
			return;

		PlayerController->AcknowledgedPawn = FortPawn;

		if (Globals::bNoMCP)
		{
			static auto HeadPart = StaticFindObject<UCustomCharacterPart>(L"/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1");
			static auto BodyPart = StaticFindObject<UCustomCharacterPart>(L"/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01");

			PlayerState->CharacterParts.Parts[0] = HeadPart;
			PlayerState->CharacterParts.Parts[1] = BodyPart;

			PlayerState->OnRep_CharacterParts();
			return;
		}

		auto& CustomizationLoadout = PlayerController->CustomizationLoadout;

		if (!CustomizationLoadout.Character || !CustomizationLoadout.Character->HeroDefinition)
			return;

		FortPawn->CustomizationLoadout = CustomizationLoadout;
		FortPawn->OnRep_CustomizationLoadout();

		PlayerState->HeroType = CustomizationLoadout.Character->HeroDefinition;
		PlayerState->OnRep_HeroType();

		ApplyCharacterCustomization(PlayerState, FortPawn);
	}

	void (*oServerReadyToStartMatch)(AFortPlayerControllerAthena* PlayerController);
	void hkServerReadyToStartMatch(AFortPlayerControllerAthena* PlayerController)
	{
		if (!PlayerController)
			return oServerReadyToStartMatch(PlayerController);

		auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		if (!PlayerState)
			return oServerReadyToStartMatch(PlayerController);

		AbilitiesHandler::ApplyAbilities(PlayerState);

		PlayerState->SquadId = (int)PlayerState->TeamIndex - 2;
		PlayerState->OnRep_PlayerTeam();
		PlayerState->OnRep_SquadId();

		return oServerReadyToStartMatch(PlayerController);
	}

	void (*oServerLoadingScreenDropped)(AFortPlayerControllerAthena* PlayerController);
	void hkServerLoadingScreenDropped(AFortPlayerControllerAthena* PlayerController)
	{
		InventoryHandler::Setup(PlayerController);

		return oServerLoadingScreenDropped(PlayerController);
	}

	void hkServerExecuteInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid)
	{
		if (!PlayerController)
			return;

		auto Pawn = static_cast<AFortPlayerPawn*>(PlayerController->Pawn);
		if (!Pawn)
			return;

		FFortItemEntry* FoundItemEntry = InventoryHandler::FindItem(PlayerController, ItemGuid);
		if (!FoundItemEntry)
			return;

		auto ItemDefinition = static_cast<UFortWeaponItemDefinition*>(FoundItemEntry->ItemDefinition);
		if (!ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
		{
			Pawn->EquipWeaponDefinition(ItemDefinition, ItemGuid);
			return;
		}

		auto DecoDefinition = static_cast<UFortDecoItemDefinition*>(ItemDefinition);
		Pawn->PickUpActor(Pawn, DecoDefinition);
		Pawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

		if (auto ContextTrapTool = reinterpret_cast<AFortDecoTool_ContextTrap*>(Pawn->CurrentWeapon))
			ContextTrapTool->ContextTrapItemDefinition = static_cast<UFortContextTrapItemDefinition*>(ItemDefinition);
	}

	void (*oEnterAircraft)(AFortPlayerControllerAthena* PlayerController, unsigned __int64 a2);
	void hkEnterAircraft(AFortPlayerControllerAthena* PlayerController, unsigned __int64 a2)
	{
		if (!PlayerController)
			return oEnterAircraft(PlayerController, a2);

		TArray<FFortItemEntry>& ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		for (int i = 0; i < ReplicatedEntries.Num(); i++)
		{
			if (static_cast<UFortWorldItemDefinition*>(ReplicatedEntries[i].ItemDefinition)->bCanBeDropped)
				InventoryHandler::RemoveItem(PlayerController, ReplicatedEntries[i].ItemGuid);
		}

		return oEnterAircraft(PlayerController, a2);
	}

	bool(__fastcall* CantBuild)(UWorld*, UObject*, FVector, FRotator, char, void*, char*) = decltype(CantBuild)(Memory::GetAddress(0xf9c0a0));
	void hkServerCreateBuildingActor(AFortPlayerControllerAthena* PlayerController, FBuildingClassData& BuildingClassData, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored)
	{
		if (!PlayerController)
			return;

		auto PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		if (!PlayerState)
			return;

		auto Class = BuildingClassData.BuildingClass.Get();
		TArray<AActor*> BuildingActorsToDestroy;
		char Result;

		if (CantBuild(Globals::GetWorld(), Class, BuildLoc, BuildRot, bMirrored, &BuildingActorsToDestroy, &Result))
		{
			BuildingActorsToDestroy.Free();
			return;
		}

		for (int i = 0; i < BuildingActorsToDestroy.Num(); i++)
		{
			BuildingActorsToDestroy[i]->K2_DestroyActor();
		}

		auto NewBuilding = Spawner::SpawnActor<ABuildingSMActor>(Class, BuildLoc, BuildRot);
		if (!NewBuilding)
			return;

		NewBuilding->InitializeKismetSpawnedBuildingActor(NewBuilding, PlayerController, true);
		NewBuilding->bPlayerPlaced = true;
		NewBuilding->Team = PlayerState->TeamIndex;
		NewBuilding->OnRep_Team();

		if (!PlayerController->bBuildFree)
			InventoryHandler::RemoveItem(PlayerController, UFortKismetLibrary::K2_GetResourceItemDefinition(NewBuilding->ResourceType), 10);
	}

	void hkServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
	{
		if (!PlayerController || !BuildingActorToEdit)
			return;

		auto Pawn = static_cast<AFortPlayerPawnAthena*>(PlayerController->Pawn);
		if (!Pawn)
			return;

		static auto EditToolDef = StaticFindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");
		if (Pawn->CurrentWeapon->WeaponData != EditToolDef)
		{
			FFortItemEntry* Entry = InventoryHandler::FindItem(PlayerController, EditToolDef);
			PlayerController->ServerExecuteInventoryItem(Entry->ItemGuid);
		}

		auto EditTool = reinterpret_cast<AFortWeap_EditingTool*>(Pawn->CurrentWeapon);
		EditTool->EditActor = BuildingActorToEdit;
		EditTool->OnRep_EditActor();
		BuildingActorToEdit->EditingPlayer = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		BuildingActorToEdit->OnRep_EditingPlayer();
	}

	static ABuildingSMActor* (*oBuildingSMActorReplaceBuildingActor)(ABuildingSMActor*, __int64, UClass*, int, int, uint8_t, AFortPlayerController*) = decltype(oBuildingSMActorReplaceBuildingActor)(Memory::GetAddress(0xdad4c0));
	void (*oServerEditBuildingActor)(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, uint8 RotationIterations, bool bMirrored);
	void hkServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, uint8 RotationIterations, bool bMirrored)
	{
		if (!PlayerController)
			return oServerEditBuildingActor(PlayerController, BuildingActorToEdit, NewBuildingClass, RotationIterations, bMirrored);

		auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		if (!PlayerState)
			return oServerEditBuildingActor(PlayerController, BuildingActorToEdit, NewBuildingClass, RotationIterations, bMirrored);

		if (BuildingActorToEdit && NewBuildingClass)
		{
			FVector BuildLocation = BuildingActorToEdit->K2_GetActorLocation();

			float HealthPercent = BuildingActorToEdit->GetHealthPercent();
			if (auto BuildingActor = oBuildingSMActorReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass, BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, PlayerController))
			{
				BuildingActor->bPlayerPlaced = true;
			}
		}

		return oServerEditBuildingActor(PlayerController, BuildingActorToEdit, NewBuildingClass, RotationIterations, bMirrored);
	}

	void hkServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing)
	{
		if (!PlayerController || !BuildingActorToStopEditing)
			return;

		auto Pawn = static_cast<APlayerPawn_Athena_C*>(PlayerController->Pawn);
		if (!Pawn)
			return;

		BuildingActorToStopEditing->EditingPlayer = nullptr;
		BuildingActorToStopEditing->OnRep_EditingPlayer();

		AFortWeap_EditingTool* EditTool = static_cast<AFortWeap_EditingTool*>(Pawn->CurrentWeapon);
		if (!EditTool)
			return;

		EditTool->bEditConfirmed = true;
		EditTool->EditActor = nullptr;
		EditTool->OnRep_EditActor();
	}

	void Initialize()
	{
		auto DefaultObject = AAthena_PlayerController_C::GetDefaultObj();

		Memory::VirtualHook(DefaultObject, 0x104, hkServerAcknowledgePossession);
		Memory::VirtualHook(DefaultObject, 0x252, hkServerReadyToStartMatch, (void**)&oServerReadyToStartMatch);
		Memory::VirtualHook(DefaultObject, 0x254, hkServerLoadingScreenDropped, (void**)&oServerLoadingScreenDropped);
		Memory::VirtualHook(DefaultObject, 0x1f4, hkServerExecuteInventoryItem);
		Memory::VirtualHook(DefaultObject, 0x212, hkServerCreateBuildingActor);
		Memory::VirtualHook(DefaultObject, 0x214, hkServerEditBuildingActor, (void**)&oServerEditBuildingActor);
		Memory::VirtualHook(DefaultObject, 0x218, hkServerBeginEditingBuildingActor);
		Memory::VirtualHook(DefaultObject, 0x216, hkServerEndEditingBuildingActor);

		MH_STATUS StatusEnterAircraft = Memory::CreateHook(Memory::GetAddress(0xcd81a0), hkEnterAircraft, (void**)&oEnterAircraft);

#ifdef LOG_HOOKSTATUS
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkEnterAircraft Status: %s.", MH_StatusToString(StatusEnterAircraft));
#endif
		

		Logging::Log(ELogEvent::Info, ELogType::Hook, "PlayerController hooks initialized.");
	}
}