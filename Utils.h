#pragma once
#include "framework.h"

namespace Utils
{
	TArray<AActor*> GetActorsOfClass(UClass* ActorClass)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(Globals::GetWorld(), ActorClass, &OutActors);

		return OutActors;
	}

	template<typename T = AActor>
	T* GetActorOfClass(UClass* ActorClass, int Index = 0)
	{
		TArray<AActor*> OutActors = GetActorsOfClass(ActorClass);
		T* Actor = reinterpret_cast<T*>(OutActors[Index]);

		OutActors.Free();
		return Actor;
	}

	AFortAlwaysRelevantReplicatedActor* GetIslandScripting()
	{
		auto Class = StaticFindObject<UClass>(L"/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C");
		if (!Class)
			return nullptr;

		return GetActorOfClass<AFortAlwaysRelevantReplicatedActor>(Class);
	}

	bool IsValidLowLevel(UObject* Object)
	{
		if (!Object || IsBadReadPtr(Object, 8))
			return false;

		if (!Object->Class || IsBadReadPtr(Object->Class, 8))
			return false;

		if (UObject::GObjects->GetByIndex(Object->Index) != Object)
			return false;

		return true;
	}

	static void ShowFoundation(ABuildingFoundation* BuildingFoundation, bool bShow = true)
	{
		if (!BuildingFoundation)
			return;

		BuildingFoundation->bServerStreamedInLevel = bShow;
		BuildingFoundation->bFoundationEnabled = bShow;
		BuildingFoundation->DynamicFoundationType = EDynamicFoundationType::Static;
		BuildingFoundation->SetDynamicFoundationEnabled(bShow);
		BuildingFoundation->OnRep_ServerStreamedInLevel();
		BuildingFoundation->OnRep_LevelToStream();
	}

	AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDefinition, int OverrideCount, int LoadedAmmo, FVector Loc, bool bTossedFromContainer = false, EFortPickupSourceTypeFlag SourceFlag = EFortPickupSourceTypeFlag::Other, AFortPawn* PawnWhoDroppedPickup = nullptr)
	{
		if (!ItemDefinition)
			return nullptr;

		auto SpawnedPickup = Spawner::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), Loc);
		if (!SpawnedPickup)
			return nullptr;

		SpawnedPickup->bRandomRotation = true;

		FFortItemEntry* PickupEntry = &SpawnedPickup->PrimaryPickupItemEntry;
		PickupEntry->ItemDefinition = ItemDefinition;
		PickupEntry->Count = OverrideCount;
		PickupEntry->LoadedAmmo = LoadedAmmo;
		PickupEntry->ReplicationKey++;
		SpawnedPickup->OnRep_PrimaryPickupItemEntry();

		SpawnedPickup->TossPickup(Loc, nullptr, 1, true, SourceFlag);

		SpawnedPickup->SetReplicateMovement(true);
		SpawnedPickup->MovementComponent = reinterpret_cast<UProjectileMovementComponent*>(UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), SpawnedPickup));

		SpawnedPickup->PawnWhoDroppedPickup = PawnWhoDroppedPickup;

		SpawnedPickup->bTossedFromContainer = bTossedFromContainer;
		SpawnedPickup->OnRep_TossedFromContainer();

		return SpawnedPickup;
	}

	AFortPickupAthena* SpawnPickup(FFortItemEntry* ItemEntry, FVector Loc, bool bTossedFromContainer = false, EFortPickupSourceTypeFlag SourceFlag = EFortPickupSourceTypeFlag::Other, AFortPawn* PawnWhoDroppedPickup = nullptr)
	{
		auto SpawnedPickup = Spawner::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), Loc);
		if (!SpawnedPickup)
			return nullptr;

		SpawnedPickup->bRandomRotation = true;
		SpawnedPickup->PrimaryPickupItemEntry = *ItemEntry;

		SpawnedPickup->TossPickup(Loc, nullptr, 1, true, SourceFlag);

		SpawnedPickup->SetReplicateMovement(true);
		SpawnedPickup->MovementComponent = reinterpret_cast<UProjectileMovementComponent*>(UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), SpawnedPickup));

		SpawnedPickup->PawnWhoDroppedPickup = PawnWhoDroppedPickup;

		SpawnedPickup->bTossedFromContainer = bTossedFromContainer;
		SpawnedPickup->OnRep_TossedFromContainer();

		return SpawnedPickup;
	}
}