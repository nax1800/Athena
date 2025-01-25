#pragma once
#include "framework.h"

namespace Server
{
	static UNetDriver* (*CreateNetDriver)(UEngine* a1, UWorld* a2, FName a3) = decltype(CreateNetDriver)(Memory::GetAddress(0x2501480));
	static bool (*InitListen)(UNetDriver* a1, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error) = decltype(InitListen)(Memory::GetAddress(0x345650));
	static void (*SetWorld)(UNetDriver* a1, UWorld* a2) = decltype(SetWorld)(Memory::GetAddress(0x22b56f0));
	static bool (*InitHost)(UObject* Beacon) = decltype(InitHost)(Memory::GetAddress(0x345270));
	static void (*PauseBeaconRequests)(UObject* Beacon, bool bPause) = decltype(PauseBeaconRequests)(Memory::GetAddress(0xd77010));
	static void (*ReplicateActors)(UReplicationDriver* a1);

	static void (*oTickFlush)(UNetDriver* a1);

	void Listen()
	{
		if (Globals::bIsServerListening)
			return;

		UWorld* World = UWorld::GetWorld();
		UNetDriver* NetDriver = nullptr;

		if (Globals::bUseBeacons)
		{
			auto Beacon = Spawner::SpawnActor<AFortOnlineBeaconHost>(AFortOnlineBeaconHost::StaticClass(), FTransform());
			if (!Beacon)
			{
				Logging::Log(ELogEvent::Error, ELogType::Athena, "Beacon failed to spawn.");
				return;
			}

			Beacon->ListenPort = Globals::Port - 1;
			InitHost(Beacon);
			PauseBeaconRequests(Beacon, false);

			NetDriver = Beacon->NetDriver;
		}
		else
		{
			NetDriver = CreateNetDriver(UEngine::GetEngine(), World, FName(282));
		}

		if (!NetDriver)
		{
			Logging::Log(ELogEvent::Error, ELogType::Athena, "NetDriver failed to create.");
			return;
		}
		else
			Logging::Log(ELogEvent::Info, ELogType::Athena, "NetDriver successfully created. (%s)", NetDriver->GetFullName().c_str());

		World->NetDriver = NetDriver;
		NetDriver->World = World;
		NetDriver->NetDriverName = FName(282);

		FString Error;
		FURL URL = FURL();
		URL.Port = Globals::Port;

		InitListen(NetDriver, World, URL, false, Error);
		SetWorld(NetDriver, World);

		World->LevelCollections[0].NetDriver = NetDriver;
		World->LevelCollections[1].NetDriver = NetDriver;
		
		auto vft = *(void***)NetDriver->ReplicationDriver;
		ReplicateActors = decltype(ReplicateActors)(vft[0x53]);

		Logging::Log(ELogEvent::Info, ELogType::Athena, "Server::Listen: Server Listening on port %i", Globals::Port);
	}

	void hkTickFlush(UNetDriver* a1)
	{
		if (a1 && a1->ReplicationDriver && a1->ClientConnections.Num() > 0 && !a1->ClientConnections[0]->InternalAck)
			Server::ReplicateActors(a1->ReplicationDriver);

		return oTickFlush(a1);
	}

	__int64 hkUWorld_GetNetMode(UWorld* a1)
	{
		return 1;
	}

	char hkKickPlayer(__int64 a1, __int64 a2, __int64 a3)
	{
		return 1;
	}

	__int64 (*oCollectGarbage)(__int64);
	__int64 hkCollectGarbage(__int64)
	{
		return 0;
	}

	static bool bMcp = false;
	__int64 hkNoMCP()
	{
		return !bMcp;
	}

	void (*oDispatchRequest)(__int64 a1, __int64* a2, int a3);
	void hkDispatchRequest(__int64 a1, __int64* a2, int a3)
	{
		*(int*)(__int64(a2) + 0x60) = 3;
		return oDispatchRequest(a1, a2, 3);
	}

	float hkGetMaxTickRate()
	{
		return 30.f;
	}

	void Initialize()
	{
		Memory::CreateHook(Memory::GetAddress(0x22b6ab0), hkTickFlush, (void**)&oTickFlush);
		Memory::CreateHook(Memory::GetAddress(0x218a4a0), hkKickPlayer);
		Memory::CreateHook(Memory::GetAddress(0x25151d0), hkCollectGarbage);
		Memory::CreateHook(Memory::GetAddress(0x2559520), hkUWorld_GetNetMode);
		Memory::CreateHook(Memory::GetAddress(0x7f2370), hkDispatchRequest, (void**)&oDispatchRequest);
		Memory::CreateHook(Memory::GetAddress(0x2508650), hkGetMaxTickRate);
	}
}

