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
    MH_STATUS StatusInitialize = MH_Initialize();
    if (StatusInitialize == MH_OK) {
        Logging::Log(ELogEvent::Info, ELogType::Hook, "Minhook successfully initialized.");
    }
    else {
        Logging::Log(ELogEvent::Error, ELogType::Hook, "Minhook failed to initialize, exiting.");
        FreeLibraryAndExitThread(GetModuleHandleA(0), 0);
    }

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);

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

