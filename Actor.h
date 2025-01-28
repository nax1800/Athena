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

    void Initialize()
    {
        MH_STATUS StatusOnDamageServer = Memory::CreateHook(Memory::GetAddress(0x14aa5f0), hkOnDamageServer, (void**)&oOnDamageServer);

#ifdef LOG_HOOKSTATUS
        Logging::Log(ELogEvent::Info, ELogType::Hook, "hkOnDamageServer Status: %s.", MH_StatusToString(StatusOnDamageServer));
#endif
        Logging::Log(ELogEvent::Info, ELogType::Hook, "Actor hooks initialized.");
    }
}