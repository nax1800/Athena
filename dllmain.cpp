#include "framework.h"

#include "Actor.h"
#include "GameMode.h"
#include "PlayerController.h"
#include "Pawn.h"
#include "Quests.h"

DWORD Initialize(LPVOID)
{
    AllocConsole();
    FILE* File;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SetConsoleTitleA("Athena - 3.6");

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
    Logging::Log(ELogEvent::Info, ELogType::Athena, "Globals::bSupportMatchmaking: %s", Globals::bSupportMatchmaking ? "true" : "false");

    Logging::Log(ELogEvent::Info, ELogType::Athena, "Globals::Port: %i", Globals::Port);

    if (Globals::bSupportMatchmaking)
    {
        *(uint8_t*)(Memory::GetAddress(0x255BB17) + 7) = 0x74;
        Logging::Log(ELogEvent::Info, ELogType::Athena, "Matchmaking issupported.");
    }

    UKismetSystemLibrary::ExecuteConsoleCommand(Globals::GetWorld(), L"open Athena_Terrain", nullptr);
    Globals::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);

    for (uintptr_t FuncToNull : vector{ 0xa767b0, 0xc22e90, 0xf1c000 })
    {
        uintptr_t func = Memory::GetAddress(FuncToNull);

        DWORD dwProtection;
        VirtualProtect((PVOID)func, 1, PAGE_EXECUTE_READWRITE, &dwProtection);

        *(uint8_t*)func = 0xC3;

        DWORD dwTemp;
        VirtualProtect((PVOID)func, 1, dwProtection, &dwTemp);
        Logging::Log(ELogEvent::Info, ELogType::Athena, "Nulled at 0x%.8x", FuncToNull);
    }

    auto ByteToPatch = (uint8_t*)(uint8_t*)(Memory::GetAddress(0x9eb786));
    DWORD dwProtection;
    VirtualProtect((PVOID)ByteToPatch, 1, PAGE_EXECUTE_READWRITE, &dwProtection);
    *ByteToPatch = 0x85;
    DWORD dwTemp;
    VirtualProtect((PVOID)ByteToPatch, 1, dwProtection, &dwTemp);
    Logging::Log(ELogEvent::Info, ELogType::Athena, "Patched Byte at 0x%.8x", ByteToPatch);

    *(bool*)Memory::GetAddress(0x4a9ca14) = false;
    Logging::Log(ELogEvent::Info, ELogType::Athena, "GIsClient should now be false.");

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

