#pragma once
#include "framework.h"

namespace Pawn
{
	void (*oServerHandlePickup)(APlayerPawn_Athena_C* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);
	void hkServerHandlePickup(APlayerPawn_Athena_C* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
	{
		if (!Pawn || !Pickup)
			return;

		if (Pickup->bPickedUp)
			return;

		auto PlayerController = static_cast<AFortPlayerControllerAthena*>(Pawn->Controller);
		if (!PlayerController)
			return;

		Pawn->IncomingPickups.Add(Pickup);
		auto& LocData = Pickup->PickupLocationData;
		LocData.StartDirection = static_cast<FVector_NetQuantizeNormal>(InStartDirection);
		LocData.FlyTime = 0.40f;
		LocData.PickupTarget = Pawn;
		LocData.ItemOwner = Pawn;
		LocData.bPlayPickupSound = bPlayPickupSound;
		LocData.PickupGuid = Pawn->CurrentWeapon ? Pawn->CurrentWeapon->ItemEntryGuid : FGuid();
		Pickup->OnRep_PickupLocationData();

		Pickup->bPickedUp = true;
		Pickup->OnRep_bPickedUp();
	}

	void (*oServerChoosePart)(AFortPlayerPawn* Pawn, EFortCustomPartType Part, UCustomCharacterPart* ChosenCharacterPart);
	void hkServerChoosePart(AFortPlayerPawn* Pawn, EFortCustomPartType Part, UCustomCharacterPart* ChosenCharacterPart)
	{
		return;
	}

	void (*oNetMulticast_Athena_BatchedDamageCues)(AFortPlayerPawn* Pawn, FAthenaBatchedDamageGameplayCues_Shared SharedData, FAthenaBatchedDamageGameplayCues_NonShared NonSharedData);
	void hkNetMulticast_Athena_BatchedDamageCues(AFortPlayerPawnAthena* Pawn, FAthenaBatchedDamageGameplayCues_Shared SharedData, FAthenaBatchedDamageGameplayCues_NonShared NonSharedData)
	{

		if (!Pawn)
			return oNetMulticast_Athena_BatchedDamageCues(Pawn, SharedData, NonSharedData);

		auto PlayerController = static_cast<AFortPlayerControllerAthena*>(Pawn->Controller);
		if (!PlayerController)
			return oNetMulticast_Athena_BatchedDamageCues(Pawn, SharedData, NonSharedData);

		auto CurrentWeapon = Pawn->CurrentWeapon;
		if (!CurrentWeapon)
			return oNetMulticast_Athena_BatchedDamageCues(Pawn, SharedData, NonSharedData);

		FFortItemEntry* CurrentItemEntry = InventoryHandler::FindItem(PlayerController, CurrentWeapon->ItemEntryGuid);
		if (!CurrentItemEntry)
			return oNetMulticast_Athena_BatchedDamageCues(Pawn, SharedData, NonSharedData);

		CurrentItemEntry->LoadedAmmo = CurrentWeapon->AmmoCount;
		PlayerController->WorldInventory->Inventory.MarkItemDirty(*CurrentItemEntry);
		PlayerController->WorldInventory->Inventory.MarkArrayDirty();

		return oNetMulticast_Athena_BatchedDamageCues(Pawn, SharedData, NonSharedData);
	}

	void Initialize()
	{
		auto DefaultObject = APlayerPawn_Athena_C::GetDefaultObj();

		MH_STATUS StatusNetMulticast_Athena_BatchedDamageCues = Memory::CreateHook(Memory::GetAddress(0x11b2b10), hkNetMulticast_Athena_BatchedDamageCues, (void**)&oNetMulticast_Athena_BatchedDamageCues);

#ifdef LOG_HOOKSTATUS
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkNetMulticast_Athena_BatchedDamageCues Status: %s.", MH_StatusToString(StatusNetMulticast_Athena_BatchedDamageCues));
#endif

		Memory::VirtualHook(DefaultObject, 0x18f, hkServerHandlePickup);
		Memory::VirtualHook(DefaultObject, 0x17f, hkServerChoosePart, (void**)&oServerChoosePart);

		Logging::Log(ELogEvent::Info, ELogType::Hook, "Pawn hooks initialized.");
	}
}