#pragma once

#include "CoreMinimal.h"
#include "ODBC/ODBC_Includes.h"
#include "ODBC_Tools.generated.h"

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

	bool operator == (const FODBC_DataValue& Other) const
	{
		return String == Other.String && Integer32 == Other.Integer32 && Integer64 == Other.Integer64 && Double == Other.Double && Boolean == Other.Boolean && DateTime == Other.DateTime && DataType == Other.DataType && DataTypeName == Other.DataTypeName && ColumnName == Other.ColumnName && Preview == Other.Preview && Note == Other.Note;
	}

	bool operator != (const FODBC_DataValue& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FODBC_DataValue& Key)
{
	uint32 Hash_String = GetTypeHash(Key.String);
	uint32 Hash_Integer32 = GetTypeHash(Key.Integer32);
	uint32 Hash_Integer64 = GetTypeHash(Key.Integer64);
	uint32 Hash_Double = GetTypeHash(Key.Double);
	uint32 Hash_Boolean = GetTypeHash(Key.Boolean);
	uint32 Hash_DateTime = GetTypeHash(Key.DateTime);
	uint32 Hash_DataType = GetTypeHash(Key.DataType);
	uint32 Hash_DataTypeName = GetTypeHash(Key.DataTypeName);
	uint32 Hash_ColumnName = GetTypeHash(Key.ColumnName);
	uint32 Hash_Preview = GetTypeHash(Key.Preview);
	uint32 Hash_Note = GetTypeHash(Key.Note);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_String);
	GenericHash = HashCombine(GenericHash, Hash_Integer32);
	GenericHash = HashCombine(GenericHash, Hash_Integer64);
	GenericHash = HashCombine(GenericHash, Hash_Double);
	GenericHash = HashCombine(GenericHash, Hash_Boolean);
	GenericHash = HashCombine(GenericHash, Hash_DateTime);
	GenericHash = HashCombine(GenericHash, Hash_DataType);
	GenericHash = HashCombine(GenericHash, Hash_DataTypeName);
	GenericHash = HashCombine(GenericHash, Hash_ColumnName);
	GenericHash = HashCombine(GenericHash, Hash_Preview);
	GenericHash = HashCombine(GenericHash, Hash_Note);
	return GenericHash;
}

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FODBC_ColumnInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Column_Name;

	UPROPERTY(BlueprintReadOnly)
	FString DataTypeName;

	UPROPERTY(BlueprintReadOnly)
	int32 DataType = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NameLenght = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DecimalDigits = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Column_Size = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNullable = 0;

	bool operator == (const FODBC_ColumnInfo& Other) const
	{
		return Column_Name == Other.Column_Name && DataTypeName == Other.DataTypeName && DataType == Other.DataType && NameLenght == Other.NameLenght && DecimalDigits == Other.DecimalDigits && Column_Size == Other.Column_Size && bIsNullable == Other.bIsNullable;
	}

	bool operator != (const FODBC_ColumnInfo& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FODBC_ColumnInfo& Key)
{
	uint32 Hash_Column_Name = GetTypeHash(Key.Column_Name);
	uint32 Hash_DataTypeName = GetTypeHash(Key.DataTypeName);
	uint32 Hash_DataType = GetTypeHash(Key.DataType);
	uint32 Hash_NameLenght = GetTypeHash(Key.NameLenght);
	uint32 Hash_DecimalDigits = GetTypeHash(Key.DecimalDigits);
	uint32 Hash_Column_Size = GetTypeHash(Key.Column_Size);
	uint32 Hash_bIsNullable = GetTypeHash(Key.bIsNullable);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_Column_Name);
	GenericHash = HashCombine(GenericHash, Hash_DataTypeName);
	GenericHash = HashCombine(GenericHash, Hash_DataType);
	GenericHash = HashCombine(GenericHash, Hash_NameLenght);
	GenericHash = HashCombine(GenericHash, Hash_DecimalDigits);
	GenericHash = HashCombine(GenericHash, Hash_Column_Size);
	GenericHash = HashCombine(GenericHash, Hash_bIsNullable);
	return GenericHash;
}

USTRUCT()
struct FF_DB_CONNECTORS_API FODBC_ResultSet
{
	GENERATED_BODY()

public:

	int64 Affected_Rows = 0;
	int64 Count_Rows = 0;
	int64 Count_Columns = 0;
	TArray<FODBC_ColumnInfo> Column_Infos;
	TMap<FVector2D, FODBC_DataValue> Data_Pool;
};

class FF_DB_CONNECTORS_API ODBC_UtilityClass final
{
public:

	// Non-instantiable utility class
	ODBC_UtilityClass() = delete;
	~ODBC_UtilityClass() = delete;
	ODBC_UtilityClass(const ODBC_UtilityClass&) = delete;
	ODBC_UtilityClass& operator=(const ODBC_UtilityClass&) = delete;

	// Static-only API (kept your exact name/signature)
	static int32 MaxColumnNameLength(SQLHDBC In_Connection);
};

USTRUCT()
struct FF_DB_CONNECTORS_API FODBC_QueryHandler
{
	GENERATED_BODY()

private:

	SQLHSTMT ODBC_Statement = nullptr;

	/*
	* This function will get data in chunks, until all data is fetched. So we don't have static buffer size limitation.
	* ColumnIndex starts from 1.
	*/ 
	FString GetChunckData(int32 ColumnIndex);

	/*
	* We don't want to iterate all columns in another "for loop", while we will already do it in "Record_Result".
	* ColumnIndex starts from 1.
	*/
	bool GetEachColumnInfo(FODBC_ColumnInfo& Out_MetaData, int32 ColumnIndex, int32 MaxColumnNameLenght);

public:

	FODBC_ResultSet ResultSet;
	bool Record_Result(FString& Out_Code, SQLHDBC In_Connection, SQLHSTMT In_Statement);

};