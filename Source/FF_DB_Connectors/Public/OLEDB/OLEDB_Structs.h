#pragma once

#include "CoreMinimal.h"
#include "OLEDB_Structs.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_ColumnUName
{
    GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
	int32 ulPropid;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
	FString pswzName;

};

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_ColumnId
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 eKind;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString eKindString;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString Guid;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
	FOLEDB_ColumnUName Column_UName;

};

USTRUCT(BlueprintType)
struct FF_DB_CONNECTORS_API FOLEDB_ColumnInfo
{
    GENERATED_BODY()

public:

    // Column display name (can be empty for some providers)
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString ColumnName;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FOLEDB_ColumnId Column_ID;

    // 1-based ordinal in the rowset
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Ordinal = 0;

    // OLE DB DBTYPE as friendly string (e.g., "DBTYPE_WSTR")
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString DataType;

    // Raw DBTYPE numeric value (useful in C++)
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 DataTypeCode = 0;

    // Max char/byte length; semantics depend on type & provider
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 ColumnSize = 0;

    // Precision/Scale for numeric types
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Precision = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Scale = 0;

    // DBCOLUMNFLAGS_* bitfield from OLE DB
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    int32 Flags = 0;

    // Human-readable flags (comma-separated for quick debugging)
    UPROPERTY(BlueprintReadOnly, Category = "Frozen Forest|Database Connectors|OLEDB")
    FString FlagsText;

};