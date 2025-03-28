#pragma once
#include "framework.h"

namespace AbilitiesHandler // for now as a symbol with this name already exists shit
{
    FGameplayAbilitySpecHandle* (*GiveAbility)(UAbilitySystemComponent* _this, FGameplayAbilitySpecHandle* outHandle, FGameplayAbilitySpec inSpec) = decltype(GiveAbility)(Memory::GetAddress(0x527580));
    __int64 (*oGiveAbilityAndActivateOnce)(void*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec) = decltype(oGiveAbilityAndActivateOnce)(Memory::GetAddress(0x5276a0));
    bool (*InternalTryActivateAbility)(UAbilitySystemComponent* _this, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, FGameplayEventData* TriggerEventData) = decltype(InternalTryActivateAbility)(Memory::GetAddress(0x528b50));
    __int64 (*SpecConstructor)(FGameplayAbilitySpec* spec, UObject* Ability, int Level, int InputID, UObject* SourceObject) = decltype(SpecConstructor)(Memory::GetAddress(0x103da30));

    FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle)
    {
        for (int i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
        {
            if (AbilitySystemComponent->ActivatableAbilities.Items[i].Handle.Handle == Handle.Handle)
            {
                return &AbilitySystemComponent->ActivatableAbilities.Items[i];
            }
        }

        return nullptr;
    }

    void hkInternalServerTryActivateAbility(UAbilitySystemComponent* ASc, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData)
    {
        FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(ASc, Handle);
        if (!Spec)
        {
            ASc->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
            return;
        }

        const UGameplayAbility* AbilityToActivate = Spec->Ability;

        UGameplayAbility* InstancedAbility = nullptr;
        Spec->InputPressed = true;

        if (!InternalTryActivateAbility(ASc, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
        {
            ASc->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
            Spec->InputPressed = false;
            ASc->ActivatableAbilities.MarkItemDirty(*Spec);
        }
    }

    FGameplayAbilitySpec* FindGameplayAbility(AFortPlayerStateAthena* PlayerState, UGameplayAbility* GameplayAbility)
    {
        auto AbilitySystemComponent = PlayerState->AbilitySystemComponent;
        if (!AbilitySystemComponent)
            return nullptr;

        for (int i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
        {
            auto& Spec = AbilitySystemComponent->ActivatableAbilities.Items[i];
            if (!Spec.Ability)
                continue;

            if (Spec.Ability == GameplayAbility)
                return &Spec;
        }

        return nullptr;
    }
    void RemoveGameplayAbility(AFortPlayerStateAthena* PlayerState, UGameplayAbility* GameplayAbility)
    {
        if (!GameplayAbility)
            return;

        auto AbilitySystemComponent = PlayerState->AbilitySystemComponent;
        if (!AbilitySystemComponent)
            return;

        FGameplayAbilitySpec* AbilitySpec = FindGameplayAbility(PlayerState, GameplayAbility);

        if (!AbilitySpec)
            return;

        AbilitySystemComponent->ClientCancelAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
        AbilitySystemComponent->ClientEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
        AbilitySystemComponent->ServerEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo, {});
    }

    void GrantGameplayAbility(AFortPlayerStateAthena* PlayerState, UClass* GameplayAbilityClass)
    {
        auto AbilitySystemComponent = PlayerState->AbilitySystemComponent;

        if (!AbilitySystemComponent)
            return;

        FGameplayAbilitySpec NewSpec{};

        SpecConstructor(&NewSpec, GameplayAbilityClass->DefaultObject, 1, -1, nullptr);
        GiveAbility(AbilitySystemComponent, &NewSpec.Handle, NewSpec);
        return;
    }

    void ApplyAbilities(AFortPlayerStateAthena* PlayerState)
    {
        static UFortAbilitySet* AbilitySet = StaticFindObject<UFortAbilitySet>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");
        TArray<TSubclassOf<UFortGameplayAbility>> GameplayAbilities = AbilitySet->GameplayAbilities;
        for (int i = 0; i < GameplayAbilities.Num(); i++)
        {
            GrantGameplayAbility(PlayerState, GameplayAbilities[i].Get());
        }
    }

    void Initialize()
    {
        auto DefaultObject = UFortAbilitySystemComponentAthena::StaticClass()->DefaultObject;

        Memory::VirtualHook(DefaultObject, 0xcb, hkInternalServerTryActivateAbility);

        Logging::Log(ELogEvent::Info, ELogType::Hook, "Abilities hooks initialized.");
    }
}