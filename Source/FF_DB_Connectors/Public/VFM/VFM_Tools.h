#pragma once

#include "CoreMinimal.h"
#include "VFM/VFM_Includes.h"
#include "VFM_Tools.generated.h"

USTRUCT()
struct FF_DB_CONNECTORS_API FVFM_Store
{
	GENERATED_BODY()

#ifdef _WIN64
	HANDLE MappingHandle = nullptr;
#endif // _WIN64

	void* MappedMemory = nullptr;
	int64 MappedSize = 0;
	FJsonObjectWrapper Headers;

};

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FVFM_Export
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> Buffer;

	UPROPERTY(BlueprintReadOnly)
	int64 MappedSize = 0;

	UPROPERTY(BlueprintReadOnly)
	FJsonObjectWrapper Headers;

};