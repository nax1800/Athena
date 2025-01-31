#pragma once
#include "framework.h"

namespace AI
{
	int BotsToSpawn = 6;
	// APlayerPawn_Athena_C
	APlayerPawn_Athena_C* SpawnBot(FVector Location)
	{
		auto Pawn = Spawner::SpawnActor<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass(), Location);
		if (!Pawn)
		{
			Logging::Log(ELogEvent::Warning, ELogType::Bot, "Failed to spawn bot.");
			return nullptr;
		}

		auto PlayerController = Spawner::SpawnActor<AAthena_PlayerController_C>(AAthena_PlayerController_C::StaticClass(), Location);
		auto PlayerState = Spawner::SpawnActor<AFortPlayerStateAthena>(AFortPlayerStateAthena::StaticClass(), Location);
		PlayerController->Player = reinterpret_cast<UPlayer*>(UGameplayStatics::SpawnObject(UPlayer::StaticClass(), PlayerController));
		PlayerController->PlayerState = PlayerState;
		PlayerController->OnRep_PlayerState();

		Pawn->PlayerState = PlayerState;
		Pawn->OnRep_PlayerState();
		Pawn->Controller = PlayerController;


		if (!Pawn->CharacterMovement)
		{
			Logging::Log(ELogEvent::Warning, ELogType::Bot, "Wild.");
			Pawn->CharacterMovement = reinterpret_cast<UCharacterMovementComponent*>(UGameplayStatics::SpawnObject(UCharacterMovementComponent::StaticClass(), Pawn));
		}

		static auto HeadPart = StaticFindObject<UCustomCharacterPart>(L"/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1");
		static auto BodyPart = StaticFindObject<UCustomCharacterPart>(L"/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01");

		PlayerState->CharacterParts.Parts[0] = HeadPart;
		PlayerState->CharacterParts.Parts[1] = BodyPart;
		PlayerState->OnRep_CharacterParts();

		PlayerState->bIsSpectator = false;
		PlayerState->bIsABot = true;

		PlayerController->Possess(Pawn);

		Pawn->PawnUniqueID = rand() % 1000;
		Pawn->OnRep_PawnUniqueID();

		static vector<FString> BotNames{
			L"SuperNax89", L"BlackSoy79", L"BlackHole9", L"GameMaster45", L"SuperPommes", L"JimmyJim3",
			L"CyberHawk22", L"ShadowByte7", L"QuantumX99", L"SteelTitan42", L"NeonPhantom5", L"NovaStorm33",
			L"EchoKnight17", L"RoboWolf66", L"PixelVortex8", L"HyperDrone12", L"TurboFalcon3", L"NanoStriker21",
			L"MechaDoom44", L"GlitchWizard88", L"CyberPhantomX", L"SolarRanger5", L"OmegaPulse11", L"AlphaStorm77",
			L"DarkPulse69", L"VortexShadow13", L"GhostRaider9", L"MechaRaptor10", L"BinaryKnight23", L"NeuralFury4",
			L"StormByteX", L"EchoHunter37", L"CosmoDroid24", L"InfinityCore6", L"PlasmaWraith2", L"HavocBlazer15",
			L"VoidSpecter99", L"CyberNexus8", L"TitanSurgeX", L"NeonSpectre19", L"StealthBot64", L"HyperRaider51",
			L"GlitchMatrix9", L"MechaPhantom0", L"AlphaSentinel7", L"ShadowDroid88", L"NightVoltX3", L"BlitzDrone9",
			L"RogueByte92", L"AeroCyber17", L"TechnoWarrior6", L"DarkStriker47", L"ZeroFusion21", L"PixelRogueX",
			L"UltraNova50", L"SolarPhantom3", L"SteelRider10", L"EchoPulse7", L"QuantumGlitch9", L"StormHawk29",
			L"NeonHunter66", L"MechaKnightX", L"InfinityVortex8", L"OmegaShadow99", L"CyberStorm23", L"NanoRaiderX",
			L"CosmoPulse7", L"DarkMatterBot4", L"VoidHunter22", L"PlasmaByte8", L"NeuralSpecter3", L"HavocStorm9",
			L"AlphaDroid77", L"BlitzSentinel12", L"HyperGlitchX", L"ShadowSurge6", L"BinaryGhost9", L"SteelSpectre5",
			L"EchoVortex11", L"NovaRogue3", L"PlasmaHawkX", L"MechaStriker4", L"QuantumSentinel2", L"StormWraith7",
			L"NeonRaider5", L"OmegaSpecter99", L"CyberRaptor12", L"NanoFusion88", L"CosmoShadow21", L"DarkVortexX",
			L"VoidPulse6", L"PlasmaRogue9", L"NeuralHunter15", L"HavocKnight2", L"AlphaStorm44", L"BlitzWraithX",
			L"HyperDroid7", L"ShadowPhantom11", L"BinaryRaider5", L"SteelHunter3", L"EchoSpectreX", L"NovaMatrix9"
		};


		if (BotNames.empty())
		{
			Logging::Log(ELogEvent::Error, ELogType::Bot, "Could not spawn a bot, ran out of names.");
			return nullptr;
		}

		int index = rand() % BotNames.size();
		FString BotName = BotNames[index];
		PlayerController->ServerChangeName(BotName);
		BotNames.erase(BotNames.begin() + index);

		PlayerState->bHasFinishedLoading = true;
		PlayerState->bHasStartedPlaying = true;
		PlayerState->OnRep_bHasStartedPlaying();
		PlayerController->ServerSetClientHasFinishedLoading(true);
		PlayerController->ServerLoadingScreenDropped();
		PlayerController->bHasInitiallySpawned = true;
		PlayerController->bAssignedStartSpawn = true;
		PlayerController->bReadyToStartMatch = true;
		PlayerController->bClientPawnIsLoaded = true;
		PlayerController->bHasClientFinishedLoading = true;
		PlayerController->bHasServerFinishedLoading = true;
		PlayerController->OnRep_bHasServerFinishedLoading();

		Pawn->SetMaxHealth(100.f);
		Pawn->SetHealth(100.f);

		auto CheatManager = reinterpret_cast<UFortCheatManager*>(UGameplayStatics::SpawnObject(UFortCheatManager::StaticClass(), PlayerController));
		PlayerController->CheatManager = CheatManager;
		CheatManager->ToggleInfiniteAmmo();

		static auto PickaxeWID = StaticFindObject<UFortWeaponMeleeItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		auto WorldItem = InventoryHandler::CreateItem(PlayerController, PickaxeWID);
		Pawn->EquipWeaponDefinition(PickaxeWID, WorldItem->GetItemGuid());

		PlayerController->ServerAcknowledgePossession(Pawn);

		EFortTeam Old = PlayerState->TeamIndex;
		PlayerState->TeamIndex = EFortTeam::HumanPvP_Team1;
		PlayerState->OnRep_TeamIndex(Old);

		PlayerController->ServerReadyToStartMatch();

		Globals::GetGameState()->PlayersLeft++;
		Globals::GetGameState()->OnRep_PlayersLeft();

		Globals::GetGameState()->PlayerArray.Add(PlayerState);
		Globals::GetGameMode()->AlivePlayers.Add(PlayerController);

		Logging::Log(ELogEvent::Info, ELogType::Bot, "Spawned bot: %s", BotName.ToString().c_str());

		return Pawn;
	}
}