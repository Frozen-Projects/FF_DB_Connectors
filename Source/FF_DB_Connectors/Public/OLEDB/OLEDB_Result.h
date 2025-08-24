#pragma once

#include "CoreMinimal.h"

#include "Generic_Includes.h"

#include "OLEDB_Result.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_ColumnInfo
{
    GENERATED_BODY()

public:

    // Column display name (can be empty for some providers)
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString ColumnName;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Column_ID_Kind;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Column_ID_Kind_String;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString Column_ID_String;

    // Raw DBTYPE numeric value (useful in C++)
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 DataType = 0;

    // OLE DB DBTYPE as friendly string (e.g., "DBTYPE_WSTR")
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString DataType_String;

    // Max char/byte length; semantics depend on type & provider
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 ColumnSize = 0;

    // Human-readable flags (comma-separated for quick debugging)
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString FlagsText;

    // 1-based ordinal in the rowset
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Ordinal = 0;

    // Precision/Scale for numeric types
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Precision = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Scale = 0;

    // DBCOLUMNFLAGS_* bitfield from OLE DB
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Flags = 0;

    bool operator == (const FOLEDB_ColumnInfo& Other) const
    {
		return ColumnName == Other.ColumnName && Column_ID_Kind == Other.Column_ID_Kind && Column_ID_Kind_String == Other.Column_ID_Kind_String && Column_ID_String == Other.Column_ID_String && DataType == Other.DataType && DataType_String == Other.DataType_String && ColumnSize == Other.ColumnSize && FlagsText == Other.FlagsText && Ordinal == Other.Ordinal && Precision == Other.Precision && Scale == Other.Scale && Flags == Other.Flags;
    }

    bool operator != (const FOLEDB_ColumnInfo& Other) const
    {
        return !(*this == Other);
    }
};

FORCEINLINE uint32 GetTypeHash(const FOLEDB_ColumnInfo& Key)
{
    uint32 Hash_ColumnName = GetTypeHash(Key.ColumnName);
    uint32 Hash_Column_ID_Kind = GetTypeHash(Key.Column_ID_Kind);
    uint32 Hash_Column_ID_Kind_String = GetTypeHash(Key.Column_ID_Kind_String);
    uint32 Hash_Column_ID_String = GetTypeHash(Key.Column_ID_String);
	uint32 Hash_DataType = GetTypeHash(Key.DataType);
	uint32 Hash_DataTypeString = GetTypeHash(Key.DataType_String);
    uint32 Hash_Column_Size = GetTypeHash(Key.ColumnSize);
	uint32 Hash_FlagsText = GetTypeHash(Key.FlagsText);
	uint32 Hash_Ordinal = GetTypeHash(Key.Ordinal);
	uint32 Hash_Precision = GetTypeHash(Key.Precision);
	uint32 Hash_Scale = GetTypeHash(Key.Scale);
	uint32 Hash_Flags = GetTypeHash(Key.Flags);

    uint32 GenericHash;
    FMemory::Memset(&GenericHash, 0, sizeof(uint32));
    GenericHash = HashCombine(GenericHash, Hash_ColumnName);
    GenericHash = HashCombine(GenericHash, Hash_Column_ID_Kind);
    GenericHash = HashCombine(GenericHash, Hash_Column_ID_Kind_String);
    GenericHash = HashCombine(GenericHash, Hash_Column_ID_String);
    GenericHash = HashCombine(GenericHash, Hash_DataType);
    GenericHash = HashCombine(GenericHash, Hash_DataTypeString);
    GenericHash = HashCombine(GenericHash, Hash_Column_Size);
    GenericHash = HashCombine(GenericHash, Hash_FlagsText);
    GenericHash = HashCombine(GenericHash, Hash_Ordinal);
    GenericHash = HashCombine(GenericHash, Hash_Precision);
	GenericHash = HashCombine(GenericHash, Hash_Scale);
	GenericHash = HashCombine(GenericHash, Hash_Flags);

    return GenericHash;
}

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

    UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
    virtual bool GetColumnData(TArray<FString>& Out_Data, int32 ColumnIndex);

};