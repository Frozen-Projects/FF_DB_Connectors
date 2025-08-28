#pragma once

#include "CoreMinimal.h"

// Custom Includes.
#include "ODBC/ODBC_Includes.h"
#include "ODBC/ODBC_Tools.h"

#include "ODBC_Result.generated.h"

UCLASS(BlueprintType)
class FF_DB_CONNECTORS_API UODBC_Result : public UObject
{
	GENERATED_BODY()

private:

	mutable FCriticalSection ResultGuard;
	FODBC_QueryHandler QueryHandler;

protected:

	// Called when the game end or when destroyed.
	virtual void BeginDestroy() override;

public:

	virtual bool SetQueryResult(FODBC_QueryHandler In_Handler);

	UFUNCTION(BlueprintPure)
	virtual int64 GetColumnNumber();

	UFUNCTION(BlueprintPure)
	virtual int64 GetRowNumber();

	UFUNCTION(BlueprintPure)
	virtual int64 GetAffectedRows();

	UFUNCTION(BlueprintPure)
	virtual FString GetQueryString();

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnInfos(FString& Out_Code, TArray<FODBC_ColumnInfo>& Out_ColumnInfo);

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnFromIndex(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, int64 Index_Column);

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnFromName(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, FString ColumName);

	UFUNCTION(BlueprintCallable)
	virtual bool GetSingleData(FString& Out_Code, FODBC_DataValue& Out_Value, FVector2D Position);

	UFUNCTION(BlueprintCallable)
	virtual bool GetRow(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, int64 Index_Row);

};