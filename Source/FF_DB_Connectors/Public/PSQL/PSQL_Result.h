#pragma once

#include "CoreMinimal.h"

// Custom Includes.
#include "PSQL/PSQL_Includes.h"

#include "PSQL_Result.generated.h"

UCLASS(BlueprintType)
class FF_DB_CONNECTORS_API UPSQL_Result : public UObject
{
	GENERATED_BODY()

private:


protected:

	// Called when the game end or when destroyed.
	virtual void BeginDestroy() override;

public:


};