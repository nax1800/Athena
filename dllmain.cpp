#include "framework.h"

#ifdef PROCESSEVENT

void (*oProcessEvent)(UObject*, UFunction*, void*);
void hkProcessEvent(UObject* InObject, UFunction* InFunction, void* InParameters)
{
    if (!InObject || !InFunction)
        return oProcessEvent(InObject, InFunction, InParameters);

    string ObjectName = InObject->GetFullName();
    string FunctionName = InFunction->GetFullName();

    if (InFunction->GetName() == "ReceiveTick" && ObjectName.contains("Athena_PlayerController"))
    {
        auto PlayerController = reinterpret_cast<AAthena_PlayerController_C*>(InObject);
        auto PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
        if (!PlayerState->bIsABot)
            return oProcessEvent(InObject, InFunction, InParameters);

        auto Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(PlayerController->Pawn);

        if (PlayerController->bHasServerFinishedLoading)
        {
            if (Globals::GetGameState()->GamePhase == EAthenaGamePhase::Setup || Globals::GetGameState()->GamePhase == EAthenaGamePhase::Warmup)
            {
                if (!PlayerController->IsPlayingEmote())
                {
                    auto bot = AI::GetBotByDisplayName(PlayerState->GetPlayerName());
                    PlayerController->ServerPlayEmoteItem(bot.Loadout.AssignedEmote);
                }
            }
            else
            {

            }
        }
    }

    return oProcessEvent(InObject, InFunction, InParameters);
}

#endif // PROCESSEVENT


DWORD Initialize(LPVOID)
{
    AllocConsole();
    FILE* File;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SetConsoleTitleA("Athena - 6.31");

    if (filesystem::exists("Athena.log"))
        filesystem::remove("Athena.log");

    Logging::Log(ELogEvent::Info, ELogType::Athena, "Made by @nax1800 and @ApfelTeeSaft.");

    MH_STATUS StatusInitialize = MH_Initialize();
    if (StatusInitialize == MH_OK) {
        Logging::Log(ELogEvent::Info, ELogType::Hook, "Minhook successfully initialized.");
    }
    else {
        Logging::Log(ELogEvent::Error, ELogType::Hook, "Minhook failed to initialize, exiting.");
        FreeLibraryAndExitThread(GetModuleHandleA(0), 0);
    }

    Logging::Log(ELogEvent::Info, ELogType::Athena, "Globals::bUseBeacons: %s", Globals::bUseBeacons ? "true" : "false");
    Logging::Log(ELogEvent::Info, ELogType::Athena, "Globals::bUseAccountID: %s", Globals::bUseAccountID ? "true" : "false");
    Logging::Log(ELogEvent::Info, ELogType::Athena, "Globals::bNoMCP: %s", Globals::bNoMCP ? "true" : "false");

    Logging::Log(ELogEvent::Info, ELogType::Athena, "Globals::Port: %i", Globals::Port);

    *(uint8_t*)(Memory::GetAddress(0x2AC2E4E) + 7) = 0x74;
    Logging::Log(ELogEvent::Info, ELogType::Athena, "Matchmaking should now be supported????");

    UKismetSystemLibrary::ExecuteConsoleCommand(Globals::GetWorld(), L"open Athena_Terrain", nullptr);
    Globals::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);

    for (uintptr_t FuncToNull : vector{ 0xd16310, 0x233bd47, 0xf89d40, 0x12f4fe0 })
    {
        uintptr_t func = Memory::GetAddress(FuncToNull);

        DWORD dwProtection;
        VirtualProtect((PVOID)func, 1, PAGE_EXECUTE_READWRITE, &dwProtection);

        *(uint8_t*)func = 0xC3;

        DWORD dwTemp;
        VirtualProtect((PVOID)func, 1, dwProtection, &dwTemp);
        Logging::Log(ELogEvent::Info, ELogType::Athena, "Nulled at 0x%.8x", FuncToNull);
    }

    auto ByteToPatch = (uint8_t*)(uint8_t*)(Memory::GetAddress(0xc98a16));
    DWORD dwProtection;
    VirtualProtect((PVOID)ByteToPatch, 1, PAGE_EXECUTE_READWRITE, &dwProtection);
    *ByteToPatch = 0x85;
    DWORD dwTemp;
    VirtualProtect((PVOID)ByteToPatch, 1, dwProtection, &dwTemp);
    Logging::Log(ELogEvent::Info, ELogType::Athena, "Patched Byte at 0x%.8x", ByteToPatch);

    *(bool*)Memory::GetAddress(0x5634b5b) = false;
    Logging::Log(ELogEvent::Info, ELogType::Athena, "GIsClient should now be false.");
    *(bool*)Memory::GetAddress(0x5634b5c) = true;
    Logging::Log(ELogEvent::Info, ELogType::Athena, "GIsServer should now be true.");

#ifdef PROCESSEVENT
    Memory::CreateHook(Memory::GetAddress(Offsets::ProcessEvent), hkProcessEvent, (void**)&oProcessEvent);
    Logging::Log(ELogEvent::Info, ELogType::Hook, "ProcessEvent is enabled.");
#endif // PROCESSEVENT

    AbilitiesHandler::Initialize();
    GameMode::Initialize();
    PlayerController::Initialize();
    Pawn::Initialize();
    QuestsHandler::Initialize();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Initialize, 0, 0, 0);
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

