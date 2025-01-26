#pragma once
#include "framework.h"

namespace InventoryHandler // for now as a symbol with this name already exists shit

{
	void Update(AFortPlayerController* PlayerController, FFortItemEntry ItemEntry)
	{
		PlayerController->WorldInventory->HandleInventoryLocalUpdate();
		PlayerController->WorldInventory->Inventory.MarkArrayDirty();
		PlayerController->WorldInventory->Inventory.MarkItemDirty(ItemEntry);
	}

	EFortQuickBars GetQuickBars(UFortItemDefinition* ItemDefinition)
	{
		if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) &&
			!ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) &&
			!ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
			return EFortQuickBars::Primary;

		return EFortQuickBars::Secondary;
	}

	bool IsFull(AFortPlayerController* PlayerController)
	{
		int Slots = 0;
		auto InstancesPtr = &PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		for (int i = 0; i < InstancesPtr->Num(); i++)
		{
			if (!InstancesPtr->operator[](i).ItemDefinition)
				continue;

			if (GetQuickBars(InstancesPtr->operator[](i).ItemDefinition) == EFortQuickBars::Primary)
			{
				Slots++;

				if (Slots >= 5)
				{
					break;
				}
			}
		}

		return Slots >= 5;
	}

	vector<FFortItemEntry*> FindItems(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition)
	{
		vector<FFortItemEntry*> FoundEntries = {};
		TArray<FFortItemEntry> ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		if (!ReplicatedEntries.IsValid())
			return FoundEntries;

		for (int i = 0; i < ReplicatedEntries.Num(); i++)
		{
			if (ReplicatedEntries[i].ItemDefinition == ItemDefinition)
			{
				auto Entry = &ReplicatedEntries[i];
				FoundEntries.push_back(Entry);
			}
		}

		return FoundEntries;
	}

	FFortItemEntry* FindItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition)
	{
		TArray<FFortItemEntry> ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		if (!ReplicatedEntries.IsValid())
			return nullptr;
		for (int i = 0; i < ReplicatedEntries.Num(); i++)
		{
			if (ReplicatedEntries[i].ItemDefinition == ItemDefinition)
				return &ReplicatedEntries[i];
		}

		return nullptr;
	}

	FFortItemEntry* FindItem(AFortPlayerController* PlayerController, FGuid ItemGuid)
	{
		TArray<FFortItemEntry> ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		if (!ReplicatedEntries.IsValid())
			return nullptr;
		for (int i = 0; i < ReplicatedEntries.Num(); i++)
		{
			if (ReplicatedEntries[i].ItemGuid == ItemGuid)
				return &ReplicatedEntries[i];
		}

		return nullptr;
	}

	UFortWorldItem* CreateItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count = 1)
	{
		auto WorldItem = static_cast<UFortWorldItem*>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1));
		WorldItem->SetOwningControllerForTemporaryItem(PlayerController);
		return WorldItem;
	}

	bool RemoveItem(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count = -1)
	{
		if (!PlayerController)
			return false;

		AFortInventory* WorldInventory = PlayerController->WorldInventory;
		if (!WorldInventory)
			return false;

		bool bWasSuccessful = false;

		for (int i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			if (WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid == ItemGuid)
			{
				if (Count == -1 || Count >= WorldInventory->Inventory.ReplicatedEntries[i].Count)
				{
					WorldInventory->Inventory.ReplicatedEntries.Remove(i);
					break;
				}

				WorldInventory->Inventory.ReplicatedEntries[i].Count -= Count;
				WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ReplicatedEntries[i]);
				bWasSuccessful = true;
			}
		}

		PlayerController->WorldInventory->Inventory.MarkArrayDirty();
		PlayerController->WorldInventory->HandleInventoryLocalUpdate();

		return bWasSuccessful;
	}

	bool RemoveItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count = -1)
	{
		if (!PlayerController || !ItemDefinition)
			return false;

		AFortInventory* WorldInventory = PlayerController->WorldInventory;
		if (!WorldInventory)
			return false;

		bool bWasSuccessful = false;

		for (int i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			if (WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDefinition)
			{
				if (Count == -1 || Count >= WorldInventory->Inventory.ReplicatedEntries[i].Count)
				{
					WorldInventory->Inventory.ReplicatedEntries.Remove(i);
					break;
				}

				WorldInventory->Inventory.ReplicatedEntries[i].Count -= Count;
				WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ReplicatedEntries[i]);
				bWasSuccessful = true;
			}
		}

		PlayerController->WorldInventory->HandleInventoryLocalUpdate();
		PlayerController->WorldInventory->Inventory.MarkArrayDirty();
		return bWasSuccessful;
	}


	FGuid AddItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDefinition, int Count = 1, int LoadedAmmo = 0, bool bForceNewItem = false)
	{
		bool bAllowMultipleStacks = ItemDefinition->bAllowMultipleStacks;

		auto MaxStackSize = ItemDefinition->MaxStackSize;

		auto& ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;

		UFortWorldItem* StackingItemInstance = nullptr;
		int OverStack = 0;

		if (MaxStackSize > 1)
		{
			for (int i = 0; i < ReplicatedEntries.Num(); i++)
			{
				auto CurrentEntry = ReplicatedEntries[i];
				auto& CurrentReplicatedEntry = CurrentEntry.ItemDefinition;

				if (CurrentEntry.ItemDefinition == ItemDefinition)
				{
					if (CurrentEntry.Count < MaxStackSize || !bAllowMultipleStacks)
					{
						StackingItemInstance = reinterpret_cast<UFortWorldItem*>(CurrentReplicatedEntry);

						OverStack = CurrentEntry.Count + Count - MaxStackSize;

						if (!bAllowMultipleStacks && !(CurrentEntry.Count < MaxStackSize))
						{
							break;
						}

						int AmountToStack = OverStack > 0 ? Count - OverStack : Count;

						auto ReplicatedEntry = FindItem(PlayerController, CurrentEntry.ItemGuid);

						CurrentEntry.Count += AmountToStack;
						ReplicatedEntry->Count += AmountToStack;

						if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()))
						{
							FFortItemEntryStateValue Value{};
							Value.IntValue = 1;
							Value.StateType = EFortItemEntryState::ShouldShowItemToast;
							ReplicatedEntry->StateValues.Add(Value);
						}

						PlayerController->WorldInventory->Inventory.MarkItemDirty(CurrentEntry);
						PlayerController->WorldInventory->Inventory.MarkItemDirty(*ReplicatedEntry);

						if (OverStack <= 0)
							return ReplicatedEntry->ItemGuid;
					}
				}
			}
		}

		Count = OverStack > 0 ? OverStack : Count;

		if (OverStack > 0 && !ItemDefinition->bAllowMultipleStacks || OverStack > 0 && IsFull(PlayerController))
		{
			auto Pawn = static_cast<APlayerPawn_Athena_C*>(PlayerController->AcknowledgedPawn);

			if (!Pawn)
				return FGuid(-1, -1, -1, -1);

			FFortItemEntry itemEntry{};
			itemEntry.Count = Count;
			itemEntry.ItemDefinition = ItemDefinition;

			// SpawnPickup(&itemEntry, Pawn->K2_GetActorLocation(), false, Pawn); i cba rn
			return FGuid(-1, -1, -1, -1);
		}

		UFortWorldItem* NewWorldItem = CreateItem(PlayerController, ItemDefinition, Count);
		if (!NewWorldItem)
		{
			return FGuid(-1, -1, -1, -1);
		}

		NewWorldItem->ItemEntry.LoadedAmmo = LoadedAmmo;
		if (ItemDefinition->MaxStackSize < Count)
		{
			PlayerController->ServerAttemptInventoryDrop(NewWorldItem->ItemEntry.ItemGuid, Count - ItemDefinition->MaxStackSize);
			NewWorldItem->ItemEntry.Count = ItemDefinition->MaxStackSize;
		}

		if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()))
		{
			FFortItemEntryStateValue Value{};
			Value.IntValue = 1;
			Value.StateType = EFortItemEntryState::ShouldShowItemToast;
			NewWorldItem->ItemEntry.StateValues.Add(Value);
		}

		PlayerController->WorldInventory->Inventory.ReplicatedEntries.Add(NewWorldItem->ItemEntry);
		PlayerController->WorldInventory->HandleInventoryLocalUpdate();
		PlayerController->WorldInventory->Inventory.MarkArrayDirty();
		PlayerController->WorldInventory->Inventory.MarkItemDirty(NewWorldItem->ItemEntry);

		return NewWorldItem->ItemEntry.ItemGuid;
	}

	void Setup(AFortPlayerControllerAthena* PlayerController)
	{
		AddItem(PlayerController, PlayerController->CustomizationLoadout.Pickaxe->WeaponDefinition, 1);

		TArray<FItemAndCount> StartingItems = Globals::GetGameMode()->StartingItems;
		for (int i = 0; i < StartingItems.Num(); i++)
		{
			FItemAndCount StartingItem = StartingItems[i];
			AddItem(PlayerController, StartingItem.Item, StartingItem.Count);
		}
	}
}