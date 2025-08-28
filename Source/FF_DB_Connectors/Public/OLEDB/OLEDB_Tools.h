#pragma once

#include "CoreMinimal.h"
#include "OLEDB_Tools.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_ColumnInfo
{
    GENERATED_BODY()

public:

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
        return Column_ID_Kind == Other.Column_ID_Kind && Column_ID_Kind_String == Other.Column_ID_Kind_String && Column_ID_String == Other.Column_ID_String && DataType == Other.DataType && DataType_String == Other.DataType_String && ColumnSize == Other.ColumnSize && FlagsText == Other.FlagsText && Ordinal == Other.Ordinal && Precision == Other.Precision && Scale == Other.Scale && Flags == Other.Flags;
    }

    bool operator != (const FOLEDB_ColumnInfo& Other) const
    {
        return !(*this == Other);
    }
};

FORCEINLINE uint32 GetTypeHash(const FOLEDB_ColumnInfo& Key)
{
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

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_Cnt_CI
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    bool bIsSuccessfull = false;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString Out_Code;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    TMap<FString, FOLEDB_ColumnInfo> Column_Infos;

};

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_Stuct_GetAll
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    bool bIsSuccessfull = false;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString Out_Code;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    TMap<FVector2D, FString> Data;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int64 Columns_Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int64 Rows_Count = 0;

};

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegate_OLEDB_CI, FOLEDB_Cnt_CI, Column_Infos);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegate_OLEDB_GetAll, FOLEDB_Stuct_GetAll, FetchResult);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegate_OLEDB_GetColumn, bool, bIsSuccessful, FString, OutCode, const TArray<FString>&, Out_Data);