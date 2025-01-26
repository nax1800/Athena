#include "framework.h"

#ifdef LOG_PROCESSEVENT

void (*oProcessEvent)(UObject*, UFunction*, void*);
void hkProcessEvent(UObject* InObject, UFunction* InFunction, void* InParameters)
{
    if (!InObject || !InFunction)
        return oProcessEvent(InObject, InFunction, InParamters);

    string ObjectName = InObject->GetFullName();
    string FunctionName = InFunction->GetFullName();

    if (!FunctionName.contains("Tick"))
    {
        Logging::Log(ELogEvent::Info, ELogType::ProcessEvent, "Object (%s)  Function (%s)", ObjectName.c_str(), FunctionName.c_str());
    }

    return oProcessEvent(InObject, InFunction, InParamters);
}

#endif // LOG_PROCESSEVENT


DWORD Initialize(LPVOID)
{
    AllocConsole();
    FILE* File;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SetConsoleTitleA("Athena - 6.31");

    Logging::Log(ELogEvent::Info, ELogType::Athena, "Made by @nax1800 and @ApfelTeeSaft.");

    MH_STATUS StatusInitialize = MH_Initialize();
    if (StatusInitialize == MH_OK) {
        Logging::Log(ELogEvent::Info, ELogType::Hook, "Minhook successfully initialized.");
    }
    else {
        Logging::Log(ELogEvent::Error, ELogType::Hook, "Minhook failed to initialize, exiting.");
        FreeLibraryAndExitThread(GetModuleHandleA(0), 0);
    }

  //  *(uint8_t*)(Memory::GetAddress(0x255BB17) + 7) = 0x74;
  //  Logging::Log(ELogEvent::Info, ELogType::Athena, "Matchmaking should now be supported.");

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);

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

#ifdef LOG_PROCESSEVENT
    Memory::CreateHook(Memory::GetAddress(Offsets::ProcessEvent), hkProcessEvent, (void**)&oProcessEvent);
    Logging::Log(ELogEvent::Info, ELogType::Hook, "ProcessEvent Logging is enabled.");
#endif // LOG_PROCESSEVENT


    GameMode::Initialize();
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

