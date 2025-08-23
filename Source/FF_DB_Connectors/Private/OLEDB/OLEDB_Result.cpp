#include "OLEDB/OLEDB_Result.h"
#include "OLEDB/OLEDB_Includes.h"

void UOLEDB_Result::BeginDestroy()
{
    // Release when UObject is destroyed
    if (RowSetBuffer)
    {
        IRowset* Rowset = reinterpret_cast<IRowset*>(RowSetBuffer);
        Rowset->Release();
        RowSetBuffer = nullptr;
    }

    Super::BeginDestroy();
}

bool UOLEDB_Result::SetRowSetBuffer(void* InRowSetBuffer)
{
    if (!InRowSetBuffer)
    {
        return false;
    }

	this->RowSetBuffer = InRowSetBuffer;
    return true;
}

void* UOLEDB_Result::GetRowSetBuffer()
{
    return this->RowSetBuffer;
}

bool UOLEDB_Result::IsValid() const
{
    return this->RowSetBuffer != nullptr;
}

FString UOLEDB_Result::DBTypeToString(unsigned short InType)
{
    switch (InType)
    {
        case DBTYPE_EMPTY:          return TEXT("DBTYPE_EMPTY");
        case DBTYPE_NULL:           return TEXT("DBTYPE_NULL");
        case DBTYPE_I2:             return TEXT("DBTYPE_I2");
        case DBTYPE_I4:             return TEXT("DBTYPE_I4");
        case DBTYPE_R4:             return TEXT("DBTYPE_R4");
        case DBTYPE_R8:             return TEXT("DBTYPE_R8");
        case DBTYPE_CY:             return TEXT("DBTYPE_CY");
        case DBTYPE_DATE:           return TEXT("DBTYPE_DATE");
        case DBTYPE_BSTR:           return TEXT("DBTYPE_BSTR");
        case DBTYPE_IDISPATCH:      return TEXT("DBTYPE_IDISPATCH");
        case DBTYPE_ERROR:          return TEXT("DBTYPE_ERROR");
        case DBTYPE_BOOL:           return TEXT("DBTYPE_BOOL");
        case DBTYPE_VARIANT:        return TEXT("DBTYPE_VARIANT");
        case DBTYPE_I1:             return TEXT("DBTYPE_I1");
        case DBTYPE_UI1:            return TEXT("DBTYPE_UI1");
        case DBTYPE_UI2:            return TEXT("DBTYPE_UI2");
        case DBTYPE_UI4:            return TEXT("DBTYPE_UI4");
        case DBTYPE_I8:             return TEXT("DBTYPE_I8");
        case DBTYPE_UI8:            return TEXT("DBTYPE_UI8");
        case DBTYPE_GUID:           return TEXT("DBTYPE_GUID");
        case DBTYPE_BYTES:          return TEXT("DBTYPE_BYTES");
        case DBTYPE_STR:            return TEXT("DBTYPE_STR");   // ANSI
        case DBTYPE_WSTR:           return TEXT("DBTYPE_WSTR");  // UTF-16
        case DBTYPE_NUMERIC:        return TEXT("DBTYPE_NUMERIC");
        case DBTYPE_DECIMAL:        return TEXT("DBTYPE_DECIMAL");
        case DBTYPE_DBTIMESTAMP:    return TEXT("DBTYPE_DBTIMESTAMP");
        case DBTYPE_DBDATE:         return TEXT("DBTYPE_DBDATE");
        case DBTYPE_DBTIME:         return TEXT("DBTYPE_DBTIME");
        default:                    return FString::Printf(TEXT("DBTYPE(0x%X)"), (unsigned)InType);
    }
}

void UOLEDB_Result::AppendFlagIfSet(FString& Out, unsigned long Flags, unsigned long Bit, const TCHAR* Name)
{
    if ((Flags & Bit) != 0)
    {
        if (!Out.IsEmpty()) Out += TEXT(", ");
        Out += Name;
    }
}

FString UOLEDB_Result::ColumnFlagsToString(unsigned long Flags)
{
    FString String;

    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_ISBOOKMARK, TEXT("BOOKMARK"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_MAYDEFER, TEXT("MAYDEFER"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_WRITE, TEXT("WRITE"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_WRITEUNKNOWN, TEXT("WRITEUNKNOWN"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_ISFIXEDLENGTH, TEXT("FIXEDLEN"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_ISNULLABLE, TEXT("NULLABLE"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_MAYBENULL, TEXT("MAYBENULL"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_ISLONG, TEXT("LONG"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_ISROWID, TEXT("ROWID"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_ISROWVER, TEXT("ROWVER"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_CACHEDEFERRED, TEXT("CACHEDEF"));
    UOLEDB_Result::AppendFlagIfSet(String, Flags, DBCOLUMNFLAGS_SCALEISNEGATIVE, TEXT("NEG_SCALE"));
    
    return String;
}

FString UOLEDB_Result::ColumnIdKindToString(int32 InKind)
{
    switch (InKind)
    {
        case DBKIND_GUID_NAME:      return TEXT("GUID_NAME");
        case DBKIND_GUID_PROPID:    return TEXT("GUID_PROPID");
        case DBKIND_NAME:           return TEXT("NAME");
	    case DBKIND_PGUID_NAME:     return TEXT("PGUID_NAME");
	    case DBKIND_PGUID_PROPID:   return TEXT("PGUID_PROPID");
	    case DBKIND_PROPID:         return TEXT("PROPID");
	    case DBKIND_GUID:           return TEXT("GUID");
        default:                    return TEXT("Unknown");
    }
}

std::string UOLEDB_Result::GuidToString(GUID guid)
{
    return std::format("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

bool UOLEDB_Result::GetColumnsInfos(TArray<FOLEDB_ColumnInfo>& OutColumnInfo)
{
    if (!this->RowSetBuffer)
    {
        return false;
    }

	TArray< FOLEDB_ColumnInfo> TempColumnInfo;
    IRowset* pRowset = reinterpret_cast<IRowset*>(RowSetBuffer);

    // Ask for column metadata
    IColumnsInfo* pColsInfo = nullptr;
    HRESULT Result = pRowset->QueryInterface(IID_IColumnsInfo, (void**)&pColsInfo);
    
    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("QueryInterface(IColumnsInfo) failed: 0x%08X"), Result);
        return false;
    }

    DBORDINAL cCols = 0;
    DBCOLUMNINFO* Columns = nullptr;
    OLECHAR* pStringsBuffer = nullptr;
    
    Result = pColsInfo->GetColumnInfo(&cCols, &Columns, &pStringsBuffer);
    pColsInfo->Release();

    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("GetColumnInfo failed: 0x%08X"), Result);
        return false;
    }

    TempColumnInfo.Reserve((int32)cCols);

    for (ULONG i = 0; i < cCols; ++i)
    {
        const DBCOLUMNINFO& Each_Column = Columns[i];

        FOLEDB_ColumnInfo Out;
        Out.ColumnName = Each_Column.pwszName ? FString(Each_Column.pwszName) : FString();
        Out.Ordinal = (int32)Each_Column.iOrdinal;
        Out.DataType = DBTypeToString(Each_Column.wType);
        Out.DataTypeCode = (int32)Each_Column.wType;
        Out.ColumnSize = (int32)Each_Column.ulColumnSize;
        Out.Precision = (int32)Each_Column.bPrecision;
        Out.Scale = (int32)Each_Column.bScale;
        Out.Flags = (int32)Each_Column.dwFlags;
        Out.FlagsText = ColumnFlagsToString(Each_Column.dwFlags);

		FOLEDB_ColumnId ColumnId;
		ColumnId.eKind = (int32)Each_Column.columnid.eKind;
		ColumnId.eKindString = UOLEDB_Result::ColumnIdKindToString((int32)Each_Column.columnid.eKind);
		
        if (Each_Column.columnid.eKind == DBKIND_GUID || Each_Column.columnid.eKind == DBKIND_PGUID_NAME || Each_Column.columnid.eKind == DBKIND_PGUID_PROPID)
        {
            const GUID Guid = *Each_Column.columnid.uGuid.pguid;
			const std::string GuidStr = UOLEDB_Result::GuidToString(Guid);
			const FString GuidFStr = FString(GuidStr.c_str());
            ColumnId.Guid = GuidFStr;
        }

        else if (Each_Column.columnid.eKind == DBKIND_GUID_NAME || Each_Column.columnid.eKind == DBKIND_GUID_PROPID || Each_Column.columnid.eKind == DBKIND_GUID)
        {
			const GUID Guid = Each_Column.columnid.uGuid.guid;
			const std::string GuidStr = UOLEDB_Result::GuidToString(Guid);
            const FString GuidFStr = FString(GuidStr.c_str());
            ColumnId.Guid = GuidFStr;
		}

        else
        {
			ColumnId.Guid = FString();
        }

		FOLEDB_ColumnUName ColumnUName;

        if (Each_Column.columnid.eKind == DBKIND_NAME || Each_Column.columnid.eKind == DBKIND_PGUID_NAME)
        {
            ColumnUName.pswzName = Each_Column.columnid.uName.pwszName ? FString(Each_Column.columnid.uName.pwszName) : FString();
        }

        else
        {
            ColumnUName.pswzName = FString();
        }

        if (Each_Column.columnid.eKind == DBKIND_PROPID || Each_Column.columnid.eKind == DBKIND_GUID_PROPID || Each_Column.columnid.eKind == DBKIND_PGUID_PROPID)
        {
            ColumnUName.ulPropid = (int32)Each_Column.columnid.uName.ulPropid;
        }

        else
        {
            ColumnUName.ulPropid = 0;
		}

		ColumnId.Column_UName = MoveTemp(ColumnUName);
		Out.Column_ID = MoveTemp(ColumnId);
        TempColumnInfo.Add(MoveTemp(Out));
    }

    // Free OLE DB allocations
    CoTaskMemFree(Columns);
    CoTaskMemFree(pStringsBuffer);

	OutColumnInfo = MoveTemp(TempColumnInfo);
    return true;
}