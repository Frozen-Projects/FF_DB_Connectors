#pragma once

#include "CoreMinimal.h"

#include "Generic_Includes.h"
#include "OLEDB/OLEDB_Tools.h"

#include "OLEDB_Result.generated.h"

UCLASS(BlueprintType)
class FF_DB_CONNECTORS_API UOLEDB_Result : public UObject
{
	GENERATED_BODY()

private:

	// Opaque IRowset* (no OLE DB headers in .h)
	void* RowSetBuffer = nullptr;

	/*
	* Helper functions.
	*/

    static FString DBTypeToString(unsigned short InType);
    static void AppendFlagIfSet(FString& Out, unsigned long Flags, unsigned long Bit, const TCHAR* Name);
    static FString ColumnFlagsToString(unsigned long Flags);
	static FString ColumnIdKindToString(int32 InKind);

protected:

	// Called when the game end or when destroyed.
	virtual void BeginDestroy() override;

public:

	/*
	* Helper functions.
	*/

    static FString GuidToString(GUID guid);
    static FString AnsiToFString(const char* AnsiStr);

	/*
	* Internal functions that can be called in a separate thread if needed.
	*/

	static FOLEDB_Cnt_CI GetColumnInfos_Internal(void* RowSetBuffer);
	static bool GetColumnFromIndex_Internal(TArray<FString>& Out_Data, FString& Out_Code, int64 ColumnIndex, void* RowBuffer);
    static FOLEDB_Stuct_GetAll GetAllData_Internal(void* RowSetBuffer);

	/*
	* Setters and getters.
	*/

	virtual bool SetRowSetBuffer(void* InRowSetBuffer);
	virtual void* GetRowSetBuffer();
	virtual bool IsValid() const;

    /*
	* @param OutColumnInfo Map of column name to column info struct. Size is column count. Key index is column index.
    */
	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual void GetColumnInfos(FDelegate_OLEDB_CI DelegateColumnInfos);

    /*
	* @param ColumnIndex is 0-based. Size of Out_Data is row count.
    */
    UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
    virtual void GetColumnFromIndex(FDelegate_OLEDB_GetColumn DelegateColumns, int64 ColumnIndex);

    UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
    virtual void GetAllData(FDelegate_OLEDB_GetAll DelegateFetch);

};