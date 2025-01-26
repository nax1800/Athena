#pragma once
#include "framework.h"

namespace PlayerController
{
	static void* (*ApplyCharacterCustomization)(void* a1, void* a2) = decltype(ApplyCharacterCustomization)(Memory::GetAddress(0x1348d00));
	void hkServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* Pawn)
	{
		auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
		auto FortPawn = static_cast<APlayerPawn_Athena_C*>(Pawn);

		PlayerController->AcknowledgedPawn = FortPawn;
		if (!PlayerState || !FortPawn)
			return;

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
		if (PlayerController)
		{
			auto PlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
			if (PlayerState)
			{
				AbilitiesHandler::Initialize();

				InventoryHandler::Setup(PlayerController);

				PlayerState->SquadId = (int)PlayerState->TeamIndex - 2;
				PlayerState->OnRep_PlayerTeam();
				PlayerState->OnRep_SquadId();
			}
		}

		return oServerReadyToStartMatch(PlayerController);
	}

	void Initialize()
	{
		auto DefaultObject = AAthena_PlayerController_C::GetDefaultObj();

		Memory::VirtualHook(DefaultObject, 0x104, hkServerAcknowledgePossession);
		Memory::VirtualHook(DefaultObject, 0x252, hkServerReadyToStartMatch, (void**)&oServerReadyToStartMatch);


		Logging::Log(ELogEvent::Info, ELogType::Hook, "PlayerController hooks initialized.");
	}
}