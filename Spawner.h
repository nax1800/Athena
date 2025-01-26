#pragma once
#include "framework.h"

namespace Spawner
{
	void sinCos(float* ScalarSin, float* ScalarCos, float Value)
	{
		float quotient = (0.31830988618f * 0.5f) * Value;
		if (Value >= 0.0f)
		{
			quotient = (float)((int)(quotient + 0.5f));
		}
		else
		{
			quotient = (float)((int)(quotient - 0.5f));
		}
		float y = Value - (2.0f * 3.1415926535897932f) * quotient;
		float sign;
		if (y > 1.57079632679f)
		{
			y = 3.1415926535897932f - y;
			sign = -1.0f;
		}
		else if (y < -1.57079632679f)
		{
			y = -3.1415926535897932f - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}
		float y2 = y * y;
		*ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;
		float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
		*ScalarCos = sign * p;
	}

	FQuat FRotToQuat(FRotator Rot)
	{
		const float DEG_TO_RAD = 3.1415926535897932f / (180.f);
		const float DIVIDE_BY_2 = DEG_TO_RAD / 2.f;
		float SP, SY, SR;
		float CP, CY, CR;

		sinCos(&SP, &CP, Rot.Pitch * DIVIDE_BY_2);
		sinCos(&SY, &CY, Rot.Yaw * DIVIDE_BY_2);
		sinCos(&SR, &CR, Rot.Roll * DIVIDE_BY_2);

		FQuat RotationQuat;
		RotationQuat.X = CR * SP * SY - SR * CP * CY;
		RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
		RotationQuat.Z = CR * CP * SY - SR * SP * CY;
		RotationQuat.W = CR * CP * CY + SR * SP * SY;

		return RotationQuat;
	}

	struct FActorSpawnParameters
	{
		FName Name = FName(0);
		AActor* Template = nullptr;
		AActor* Owner = nullptr;
		APawn* Instigator = nullptr;
		ULevel* OverrideLevel = nullptr;
		ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
		uint16	bRemoteOwned : 1;
		uint16	bNoFail : 1;
		uint16	bDeferConstruction : 1;
		uint16	bAllowDuringConstructionScript : 1;
		EObjectFlags ObjectFlags;
	};


	FActorSpawnParameters CreateSpawnParams(ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, bool bDeferConstruction = false, AActor* Owner = nullptr)
	{
		FActorSpawnParameters Params{};
		Params.Owner = Owner;
		Params.SpawnCollisionHandlingOverride = SpawnCollisionHandlingOverride;
		Params.bDeferConstruction = bDeferConstruction;
		return Params;
	}

	template<typename T = AActor>
	T* SpawnActor(UClass* ActorClass, FTransform SpawnTransform = FTransform(), FActorSpawnParameters SpawnParameters = CreateSpawnParams())
	{
		static auto oSpawnActor = (AActor* (*)(UWorld * World, UClass * Class, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters))(Memory::GetAddress(0x276a290));
		if (!oSpawnActor)
			return nullptr;

		auto Actor = (T*)oSpawnActor(UWorld::GetWorld(), ActorClass, &SpawnTransform, SpawnParameters);
		return Actor;
	}

	template<typename T = AActor>
	T* SpawnActor(UClass* ActorClass, FVector Location = {}, FRotator Rotation = {}, FActorSpawnParameters SpawnParameters = CreateSpawnParams())
	{
		FTransform SpawnTransform{};
		SpawnTransform.Translation = Location;
		SpawnTransform.Scale3D = { 1,1,1 };
		SpawnTransform.Rotation = FRotToQuat(Rotation);

		auto Actor = SpawnActor<T>(ActorClass, SpawnTransform, SpawnParameters);

		VirtualFree(&SpawnParameters, 0, MEM_RELEASE);

		return Actor;
	}

/*	template<typename T = AActor>
	T* SpawnActor(UClass* ActorClass, FVector Location = {}, FRotator Rotation = {}, AActor* Owner = nullptr)
	{
		FTransform Transform;

		Transform.Translation = Location;
		Transform.Scale3D = { 1,1,1 };
		Transform.Rotation = FRotToQuat(Rotation);

		AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), ActorClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
		UGameplayStatics::FinishSpawningActor(Actor, Transform);

		return static_cast<T*>(Actor);
	}

	template<typename T = AActor>
	T* SpawnActor(FVector Location = {}, FRotator Rotation = {}, AActor* Owner = nullptr)
	{
		return SpawnActor<T>(T::StaticClass());
	}*/
}