#pragma once

#include "CoreMinimal.h"

// Custom Includes.
#include "ODBC/ODBC_Includes.h"

#include "ODBC_Result.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FODBC_DataValue
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString String;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	int32 Integer32 = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	int64 Integer64 = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	float Double = (double)0.f;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	bool Boolean = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FDateTime DateTime;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	int32 DataType = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString DataTypeName;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString ColumnName;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString Preview;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString Note;

};

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FODBC_ColumnInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Column_Name;

	UPROPERTY(BlueprintReadOnly)
	int32 NameLenght = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DataType = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DecimalDigits = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNullable = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Column_Size = 0;
};

USTRUCT()
struct FF_DB_CONNECTORS_API FODBC_QueryHandler
{
	GENERATED_BODY()

private:

	FString GetChunckData(int32 SQL_Column_Index);

public:

	SQLHSTMT SQL_Handle = nullptr;

	FString SentQuery;
	int64 Affected_Rows = 0;
	int64 Count_Rows = 0;
	int64 Count_Columns = 0;
	TMap<FVector2D, FODBC_DataValue> Data_Pool;

	bool GetEachColumnInfo(FODBC_ColumnInfo& Out_MetaData, int32 ColumnIndex);
	
	// 0 = Failed, 1 = Success , 2 = Update Only.
	int32 Record_Result(FString& Out_Code);

};

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