#pragma once

#include "CoreMinimal.h"

#include "Generic_Structs.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FDB_Data_Key
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors")
	FString ColumnName;

	UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors")
	int64 RowIndex;

};