#pragma once

#include <windows.h>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <numeric>

#include "MinHook/include/MinHook.h"
#include "SDK.hpp"

using namespace std;
using namespace SDK;

#define PROCESSEVENT
#define LOG_HOOKSTATUS


#include "Memory.h"
#include "Logging.h"

#include "Frame.h"

template <typename T>
T* StaticFindObject(wstring ObjectName, UClass* ObjectClass = nullptr)
{
	static UObject* (*oStaticFindObject)(UClass * Class, void* InOuter, const wchar_t* Name, bool ExactClass) = decltype(oStaticFindObject)(Memory::GetAddress(0x19d5680));
	return (T*)oStaticFindObject(ObjectClass, nullptr, ObjectName.c_str(), false);
}

template <typename T>
static T* StaticLoadObject(wstring ObjectName, UClass* ObjectClass = nullptr)
{ 
	static UObject* (*oStaticLoadObject)(UObject*, UObject*, const TCHAR*, const TCHAR*, uint32_t, void*, bool) = decltype(oStaticLoadObject)(Memory::GetAddress(0x19d70f0));
	return reinterpret_cast<T*>(oStaticLoadObject(ObjectClass, 0, ObjectName.c_str(), nullptr, 0, nullptr, false));
}

#include "Globals.h"

#include "Spawner.h"

#include "Utils.h"

#include "Inventory.h"
#include "Abilities.h"

#include "AI.h"

#include "Server.h"

#include "Looting.h"

#include "Actor.h"
#include "GameMode.h"
#include "PlayerController.h"
#include "Pawn.h"
#include "Quests.h"
