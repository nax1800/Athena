#pragma once
#include "framework.h"

namespace Server
{
	static UNetDriver* (*CreateNetDriver)(UEngine* a1, UWorld* a2, FName a3) = decltype(CreateNetDriver)(Memory::GetAddress(0x2A56600));
	static bool (*InitListen)(UNetDriver* a1, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error) = decltype(InitListen)(Memory::GetAddress(0x44b700));
	static void (*SetWorld)(UNetDriver* a1, UWorld* a2) = decltype(SetWorld)(Memory::GetAddress(0x27e2040));
	static bool (*InitHost)(UObject* Beacon) = decltype(InitHost)(Memory::GetAddress(0x44b320));
	static void (*PauseBeaconRequests)(UObject* Beacon, bool bPause) = decltype(PauseBeaconRequests)(Memory::GetAddress(0x1149920));
	static void (*ReplicateActors)(UReplicationDriver* a1);

	void Listen()
	{
		if (Globals::bIsServerListening)
			return;

		Globals::bIsServerListening = true;

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
			NetDriver = CreateNetDriver(UEngine::GetEngine(), World, FName(282));

		if (!NetDriver)
		{
			Logging::Log(ELogEvent::Error, ELogType::Athena, "NetDriver failed to create.");
			return;
		}
			
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
		ReplicateActors = decltype(ReplicateActors)(vft[0x56]);

		if (NetDriver->MaxInternetClientRate < NetDriver->MaxClientRate && NetDriver->MaxInternetClientRate > 2500)
			NetDriver->MaxClientRate = NetDriver->MaxInternetClientRate;

		Logging::Log(ELogEvent::Info, ELogType::Athena, "Server::Listen: Server Listening on port %i", Globals::Port);
	}

	APlayerPawn_Athena_C* Bot = nullptr;

	void (*oTickFlush)(UNetDriver* a1);
	void hkTickFlush(UNetDriver* a1)
	{
		if (a1 && a1->ReplicationDriver && a1->ClientConnections.Num() > 0 && !a1->ClientConnections[0]->InternalAck)
			Server::ReplicateActors(a1->ReplicationDriver);

		if (GetAsyncKeyState(VK_F6) & 0x01)
		{
			auto Pawn = Utils::GetActorOfClass<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass());
			if (Pawn)
			{
				Bot = AI::SpawnBot(Pawn->K2_GetActorLocation() + FVector(10, 0, 10));
			}
		}

		return oTickFlush(a1);
	}

	__int64 hkGetNetMode(UWorld*)
	{
		return 1;
	}

	char hkKickPlayer(__int64 a1, __int64 a2, __int64 a3)
	{
		return 1;
	}

	__int64 hkCollectGarbage(__int64)
	{
		return 0;
	}

	__int64 hkNoMCP()
	{
		return Globals::bNoMCP;
	}

	void (*oDispatchRequest)(__int64 a1, __int64* a2, int a3);
	void hkDispatchRequest(__int64 a1, __int64* a2, int a3)
	{
		if (Globals::bNoMCP)
			return oDispatchRequest(a1, a2, a3);

		*(int*)(__int64(a2) + 0x28) = 3;
		return oDispatchRequest(a1, a2, 3);
	}

	float hkGetMaxTickRate()
	{
		return 30.f;
	}

	void Initialize()
	{
		Memory::CreateHook(Memory::GetAddress(0x27e36a0), hkTickFlush, (void**)&oTickFlush);
		Memory::CreateHook(Memory::GetAddress(0x26a2e00), hkKickPlayer);
		Memory::CreateHook(Memory::GetAddress(0x5f4cc22), hkCollectGarbage);
		Memory::CreateHook(Memory::GetAddress(0x35b57b76), hkGetNetMode);
		// Memory::CreateHook(Memory::GetAddress(0x2470ca0), hkActorGetNetMode);
		Memory::CreateHook(Memory::GetAddress(0x96e200), hkDispatchRequest, (void**)&oDispatchRequest);
		Memory::CreateHook(Memory::GetAddress(0x2a62bb0), hkGetMaxTickRate);
		Memory::CreateHook(Memory::GetAddress(0xfb7e40), hkNoMCP);
	}
}

