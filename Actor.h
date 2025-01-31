#pragma once
#include "framework.h"

namespace ActorHandler
{
    void (*oOnDamageServer)(ABuildingActor* Actor, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AActor* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext);
    void hkOnDamageServer(ABuildingActor* Actor, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AActor* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext)
    {
        if (!Actor || !InstigatedBy || !DamageCauser)
            return oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

        if (!DamageCauser->IsA(AFortWeapon::StaticClass()))
            return oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

        auto BuildingActor = static_cast<ABuildingSMActor*>(Actor);
        auto Weapon = static_cast<AFortWeapon*>(DamageCauser);
        auto PlayerController = static_cast<AFortPlayerControllerAthena*>(InstigatedBy);

        if (!Weapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
            return oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

        auto PlayerPawn = static_cast<AFortPlayerPawn*>(PlayerController->Pawn);
        if (!PlayerPawn)
            return oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

        if (BuildingActor->bDestroyed || BuildingActor->bPlayerPlaced)
            return oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

        bool bWeakSpotHit = (Damage == 100.f);
        auto ResourceDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->ResourceType);
        if (!ResourceDefinition)
            return  oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

        FCurveTableRowHandle BuildingResourceAmountOverride = BuildingActor->BuildingResourceAmountOverride;

        int ResourceAmount = 0;

        if (BuildingResourceAmountOverride.RowName.ComparisonIndex)
        {
            static UFortPlaylistAthena* Playlist = Globals::GetGameState()->CurrentPlaylistInfo.OverridePlaylist;
            static UCurveTable* ResourceRates = Playlist->ResourceRates.Get();

            if (!ResourceRates)
            {
                ResourceRates = StaticFindObject<UCurveTable>(L"/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");
                Logging::Log(ELogEvent::Warning, ELogType::Athena, "Playlist's ResourceRate was null, set to 'AthenaResourceRates'.");
            }

            float AmountOut = 0;

            UDataTableFunctionLibrary::EvaluateCurveTableRow(ResourceRates, BuildingResourceAmountOverride.RowName, 0.f, nullptr, &AmountOut, L"");

            float Amount = AmountOut / (BuildingActor->GetMaxHealth() / Damage);
            ResourceAmount = round(Amount);
        }

        if (ResourceAmount > 0)
        {
            PlayerController->ClientReportDamagedResourceBuilding(BuildingActor, BuildingActor->ResourceType, ResourceAmount, Actor->bDestroyed, bWeakSpotHit);
            InventoryHandler::AddItem(PlayerController, ResourceDefinition, ResourceAmount);
        }

        return oOnDamageServer(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
    }

	char (*oCompletePickupAnimation)(AFortPickup* Pickup);
	char hkCompletePickupAnimation(AFortPickup* Pickup)
	{
		if(!Pickup)
			return oCompletePickupAnimation(Pickup);

		auto Pawn = static_cast<APlayerPawn_Athena_C*>(Pickup->PickupLocationData.PickupTarget);

		if (!Pawn)
			return oCompletePickupAnimation(Pickup);

		auto PlayerController = static_cast<AFortPlayerControllerAthena*>(Pawn->GetController());

		if (!PlayerController)
			return oCompletePickupAnimation(Pickup);

		FGuid Swap = FGuid(-1, -1, -1, -1);

		FFortItemEntry* PickupEntry = &Pickup->PrimaryPickupItemEntry;
		FFortItemEntry* InvItemEntry = InventoryHandler::FindItem(PlayerController, Pickup->PickupLocationData.PickupGuid);

		if (!PickupEntry || !InvItemEntry)
		{
			FFortItemEntry DropEntry{};
			DropEntry.Count = PickupEntry->Count;
			DropEntry.ItemDefinition = PickupEntry->ItemDefinition;
			DropEntry.LoadedAmmo = PickupEntry->LoadedAmmo;

			Utils::SpawnPickup(&DropEntry, Pawn->K2_GetActorLocation(), false, EFortPickupSourceTypeFlag::Player, Pawn);

			return oCompletePickupAnimation(Pickup);
		}

		auto PickupItemDefinition = static_cast<UFortWorldItemDefinition*>(PickupEntry->ItemDefinition);
		int IncomingCount = PickupEntry->Count;
		FGuid ItemGuid = PickupEntry->ItemGuid;

		if (!PickupItemDefinition)
			return oCompletePickupAnimation(Pickup);

		if (InventoryHandler::GetQuickBars(PickupItemDefinition) == EFortQuickBars::Primary)
		{
			int NewCount = 0;
			int OverStack = 0;

			bool bSuccess = false;
			bool bDrop = false;

			for (int i = 0; i < PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
			{
				FFortItemEntry* CurrentEntry = &PlayerController->WorldInventory->Inventory.ReplicatedEntries[i];
				if (CurrentEntry->ItemDefinition == PickupItemDefinition)
				{
					NewCount = CurrentEntry->Count + IncomingCount;
					OverStack = NewCount - PickupItemDefinition->MaxStackSize;

					CurrentEntry->Count = OverStack > 0 ? NewCount - OverStack : NewCount;

					PlayerController->WorldInventory->Inventory.MarkItemDirty(*CurrentEntry);
					PlayerController->WorldInventory->Inventory.MarkArrayDirty();

					if (OverStack <= 0)
					{
						bSuccess = true;
						break;
					}
				}
			}

			bDrop = !bSuccess;

			if (InventoryHandler::IsFull(PlayerController) && bDrop)
			{
				int DropCount = OverStack > 0 ? OverStack : IncomingCount;

				if (static_cast<UFortWorldItemDefinition*>(InvItemEntry->ItemDefinition)->bCanBeDropped && InvItemEntry->ItemDefinition->bAllowMultipleStacks)
				{
					PlayerController->ServerAttemptInventoryDrop(InvItemEntry->ItemGuid, InvItemEntry->Count);
					Swap = InventoryHandler::AddItem(PlayerController, PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo);
				}
				else
				{
					FFortItemEntry DropEntry{};
					DropEntry.Count = DropCount;
					DropEntry.ItemDefinition = PickupItemDefinition;
					DropEntry.LoadedAmmo = PickupEntry->LoadedAmmo;

					Utils::SpawnPickup(&DropEntry, Pawn->K2_GetActorLocation(), false, EFortPickupSourceTypeFlag::Player, Pawn);
				}
			}
			else if (!InventoryHandler::IsFull(PlayerController) && bDrop)
			{
				int DropCount = OverStack > 0 ? OverStack : IncomingCount;

				InventoryHandler::AddItem(PlayerController, PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo);
			}
		}
		else
			InventoryHandler::AddItem(PlayerController, PickupItemDefinition, IncomingCount, PickupEntry->LoadedAmmo);


		FGuid CurrentGuid = FGuid(-1, -1, -1, -1);
		if (Pawn->CurrentWeapon)
			CurrentGuid = Pawn->CurrentWeapon->ItemEntryGuid;

		if (Swap != FGuid(-1, -1, -1, -1) && CurrentGuid == Pickup->PickupLocationData.PickupGuid)
			PlayerController->ClientEquipItem(Swap, true);

		return oCompletePickupAnimation(Pickup);
	}

    void Initialize()
    {
        MH_STATUS StatusOnDamageServer = Memory::CreateHook(Memory::GetAddress(0x14aa5f0), hkOnDamageServer, (void**)&oOnDamageServer);
		MH_STATUS StatusCompletePickupAnimation = Memory::CreateHook(Memory::GetAddress(0x109d390), hkCompletePickupAnimation, (void**)&oCompletePickupAnimation);

#ifdef LOG_HOOKSTATUS
        Logging::Log(ELogEvent::Info, ELogType::Hook, "hkOnDamageServer Status: %s.", MH_StatusToString(StatusOnDamageServer));
		Logging::Log(ELogEvent::Info, ELogType::Hook, "hkCompletePickupAnimation Status: %s.", MH_StatusToString(StatusCompletePickupAnimation));
#endif
        Logging::Log(ELogEvent::Info, ELogType::Hook, "Actor hooks initialized.");
    }
}