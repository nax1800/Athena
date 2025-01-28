#pragma once
#include "framework.h"

namespace Utils
{
	template<typename T = AActor>
	T* GetActorOfClass(UClass* ActorClass, int Index = 0)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(Globals::GetWorld(), ActorClass, &OutActors);
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
}