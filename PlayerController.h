#pragma once
#include "framework.h"

namespace PlayerController
{
	static void* (*ApplyCharacterCustomization)(void* a1, void* a2) = decltype(ApplyCharacterCustomization)(Memory::GetAddress(0x1348d00));
	void hkServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* Pawn)
	{
		auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		auto FortPawn = static_cast<AFortPlayerPawnAthena*>(Pawn);

		PlayerController->AcknowledgedPawn = FortPawn;
		if (!PlayerState || !FortPawn)
			return;

		// TArray<FFortItemEntry>& CustomizationLoadout = PlayerController->CustomizationLoadout;

		//if (!CustomizationLoadout.Character || !CustomizationLoadout.Character->HeroDefinition)
		//	return;

		// not needed ig?
		// FortPawn->CustomizationLoadout = CustomizationLoadout;
		// FortPawn->OnRep_CustomizationLoadout();

		// PlayerState->HeroType = CustomizationLoadout.Character->HeroDefinition;
		PlayerState->OnRep_HeroType();

		ApplyCharacterCustomization(PlayerState, FortPawn);
	}

	void (*oServerReadyToStartMatch)(AFortPlayerControllerAthena* PlayerController);
	void hkServerReadyToStartMatch(AFortPlayerControllerAthena* PlayerController)
	{
		if (PlayerController)
		{
			auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
			if (PlayerState)
			{
				AbilitiesHandler::ApplyAbilities(PlayerState);

				PlayerState->SquadID = (int)PlayerState->TeamIndex - 2;
				PlayerState->OnRep_PlayerTeam();
				PlayerState->OnRep_SquadId();
			}
		}

		return oServerReadyToStartMatch(PlayerController);
	}

	void (*oServerLoadingScreenDropped)(AFortPlayerControllerAthena* PlayerController);
	void hkServerLoadingScreenDropped(AFortPlayerControllerAthena* PlayerController)
	{
		// InventoryHandler::Setup(PlayerController);

		return oServerLoadingScreenDropped(PlayerController);
	}

	//void hkServerExecuteInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid, FGuid TrackerGuid)
	//{
	//	FFortItemEntry* FoundItemEntry = InventoryHandler::FindItem(PlayerController, ItemGuid);

	//	auto Pawn = static_cast<AFortPlayerPawn*>(PlayerController->Pawn);
	//	if (!Pawn)
	//		return;

	//	if (!FoundItemEntry)
	//		return;

	//	auto ItemDefinition = static_cast<UFortWeaponItemDefinition*>(FoundItemEntry->ItemDefinition);
	//	if (ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
	//	{
	//		auto DecoDefinition = static_cast<UFortDecoItemDefinition*>(ItemDefinition);
	//		Pawn->PickUpActor(Pawn, DecoDefinition);
	//		Pawn->CurrentWeapon->ItemEntryGuid = ItemGuid;
	//		Pawn->CurrentWeapon->TrackerGuid = TrackerGuid;

	//		if (auto ContextTrapTool = reinterpret_cast<AFortDecoTool_ContextTrap*>(Pawn->CurrentWeapon))
	//			ContextTrapTool->ContextTrapItemDefinition = static_cast<UFortContextTrapItemDefinition*>(ItemDefinition);

	//		return;
	//	}

	//	Pawn->EquipWeaponDefinition(ItemDefinition, ItemGuid, TrackerGuid, false);
	//}

	void (*oEnterAircraft)(AFortPlayerControllerAthena* PlayerController, unsigned __int64 a2);
	void hkEnterAircraft(AFortPlayerControllerAthena* PlayerController, unsigned __int64 a2)
	{
		//TArray<FFortItemEntry>& ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		//for (int i = 0; i < ReplicatedEntries.Num(); i++)
		//{
		//	if (static_cast<UFortWorldItemDefinition*>(ReplicatedEntries[i].ItemDefinition)->bCanBeDropped)
		//	InventoryHandler::RemoveItem(PlayerController, ReplicatedEntries[i].ItemGuid);
		//}

		return oEnterAircraft(PlayerController, a2);
	}

	void Initialize()
	{
		auto DefaultObject = AFortPlayerControllerAthena::GetDefaultObj();

		Memory::VirtualHook(DefaultObject, 0x104, hkServerAcknowledgePossession);
		Memory::VirtualHook(DefaultObject, 0x252, hkServerReadyToStartMatch, (void**)&oServerReadyToStartMatch);
		Memory::VirtualHook(DefaultObject, 0x254, hkServerLoadingScreenDropped, (void**)&oServerLoadingScreenDropped);
		Memory::VirtualHook(DefaultObject, 0x1f4, hkServerExecuteInventoryItem);

		MH_STATUS StatusEnterAircraft = Memory::CreateHook(Memory::GetAddress(0xcd81a0), hkEnterAircraft, (void**)&oEnterAircraft);

#ifdef LOG_HOOKSTATUS
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkEnterAircraft Status: %s.", MH_StatusToString(StatusEnterAircraft));
#endif
		

		Logging::Log(ELogEvent::Info, ELogType::Hook, "PlayerController hooks initialized.");
	}
}