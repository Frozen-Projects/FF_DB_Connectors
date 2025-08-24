#pragma once

#include "CoreMinimal.h"

#include "Generic_Includes.h"
#include "Generic_Structs.h"
#include "OLEDB/OLEDB_Structs.h"

#include "OLEDB_Result.generated.h"

UCLASS(BlueprintType)
class FF_DB_CONNECTORS_API UOLEDB_Result : public UObject
{
	GENERATED_BODY()

private:

	// Opaque IRowset* (no OLE DB headers in .h)
	void* RowSetBuffer = nullptr;

    static FString DBTypeToString(unsigned short InType);
    static void AppendFlagIfSet(FString& Out, unsigned long Flags, unsigned long Bit, const TCHAR* Name);
    static FString ColumnFlagsToString(unsigned long Flags);
	static FString ColumnIdKindToString(int32 InKind);
	static FString GuidToString(GUID guid);

protected:

	// Called when the game end or when destroyed.
	virtual void BeginDestroy() override;

public:

	virtual bool SetRowSetBuffer(void* InRowSetBuffer);
	virtual void* GetRowSetBuffer();
	virtual bool IsValid() const;

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual bool GetColumnsInfos(TArray<FOLEDB_ColumnInfo>& OutColumnInfo);

};