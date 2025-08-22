#pragma once

#include "CoreMinimal.h"

#include "OLEDB_Result.generated.h"

UCLASS(BlueprintType)
class FF_DB_CONNECTORS_API UOLEDB_Result : public UObject
{
	GENERATED_BODY()

private:

	// Opaque IRowset* (no OLE DB headers in .h)
	void* RowSetBuffer = nullptr;

protected:

	// Called when the game end or when destroyed.
	virtual void BeginDestroy() override;

public:

	virtual bool SetRowSetBuffer(void* InRowSetBuffer);
	virtual void* GetRowSetBuffer();
	virtual bool IsValid() const;

};