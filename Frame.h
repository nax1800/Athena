#pragma once
#include "framework.h"

#define RESULT_DECL void*const RESULT_PARAM

class FOutputDevice
{
public:
	bool bSuppressEventTag;
	bool bAutoEmitLineTerminator;
	uint8_t _Padding1[0x6];
};

class FFrame : public FOutputDevice
{
public:
	void** VTable;
	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;
	UProperty* MostRecentProperty;
	uint8* MostRecentPropertyAddress;
	FFrame* PreviousFrame;
	UField* PropertyChainForCompiledIn;
	UFunction* CurrentNativeFunction;

	bool bArrayContextFailed;

public:
	__forceinline void StepExplicitProperty(void* const Result, void* Property)
	{
		static void (*StepExplicitPropertyOriginal)(__int64 frame, void* const Result, void* Property) = decltype(StepExplicitPropertyOriginal)(Memory::GetAddress(0x19b0980));
		StepExplicitPropertyOriginal(__int64(this), Result, Property);
	}

	__forceinline void Step(UObject* Context, RESULT_DECL)
	{
		static void (*StepOriginal)(__int64 frame, UObject * Context, RESULT_DECL) = decltype(StepOriginal)(Memory::GetAddress(0x19b0950));
		StepOriginal(__int64(this), Context, RESULT_PARAM);
	}

	__forceinline void StepCompiledIn(void* const Result)
	{
		if (Code)
		{
			Step(Object, Result);
		}
		else
		{
			PropertyChainForCompiledIn = PropertyChainForCompiledIn->Next;
			StepExplicitProperty(Result, PropertyChainForCompiledIn);
		}
	}

	template<typename TNativeType>
	__forceinline TNativeType& StepCompiledInRef(void* const TemporaryBuffer)
	{
		MostRecentPropertyAddress = nullptr;

		if (Code)
			Step(Object, TemporaryBuffer);

		return (MostRecentPropertyAddress != NULL) ? *(TNativeType*)(MostRecentPropertyAddress) : *(TNativeType*)TemporaryBuffer;
	}
};