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

FString UOLEDB_Result::GuidToString(GUID guid)
{
    const std::string Result = std::format("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return FString(Result.c_str());
}

FString UOLEDB_Result::AnsiToFString(const char* AnsiStr)
{
    if (!AnsiStr)
    {
        return FString();
    }

    // Get required buffer size for conversion
    int32 RequiredSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, AnsiStr, -1, nullptr, 0);

    if (RequiredSize <= 0)
    {
        return FString();
    };

    // Allocate buffer
    TArray<WCHAR> WideBuffer;
    WideBuffer.SetNumUninitialized(RequiredSize);

    MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, AnsiStr, -1, WideBuffer.GetData(), RequiredSize);

    return FString(WideBuffer.GetData());
}

bool UOLEDB_Result::ResetCursor()
{
    if (!this->RowSetBuffer)
    {
        return false;
    }

    IRowset* pRowset = reinterpret_cast<IRowset*>(RowSetBuffer);

    if (!pRowset)
    {
        return false;
    }

    HRESULT Result = pRowset->RestartPosition(DB_NULL_HCHAPTER);

    if (SUCCEEDED(Result))
    {
        return true;
    }

    else
    {
        return false;
    }
}

bool UOLEDB_Result::GetColumnsInfos(TArray<FOLEDB_ColumnInfo>& OutColumnInfo)
{
    if (!this->RowSetBuffer)
    {
        return false;
    }

    IRowset* pRowset = reinterpret_cast<IRowset*>(RowSetBuffer);

    if (!pRowset)
    {
        return false;
	}

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

    TArray< FOLEDB_ColumnInfo> Array_Columns_Infos;
    Array_Columns_Infos.Reserve((int32)cCols);

    for (ULONG i = 0; i < cCols; ++i)
    {
        const DBCOLUMNINFO& Each_Column = Columns[i];

        FOLEDB_ColumnInfo Each_Column_Info;
        Each_Column_Info.ColumnName = Each_Column.pwszName ? FString(Each_Column.pwszName) : FString();
        Each_Column_Info.Ordinal = (int32)Each_Column.iOrdinal;
        Each_Column_Info.DataType_String = DBTypeToString(Each_Column.wType);
        Each_Column_Info.DataType = (int32)Each_Column.wType;
        Each_Column_Info.ColumnSize = (int32)Each_Column.ulColumnSize;
        Each_Column_Info.Precision = (int32)Each_Column.bPrecision;
        Each_Column_Info.Scale = (int32)Each_Column.bScale;
        Each_Column_Info.Flags = (int32)Each_Column.dwFlags;
        Each_Column_Info.FlagsText = ColumnFlagsToString(Each_Column.dwFlags);

		const int32 eKind = (int32)Each_Column.columnid.eKind;
		Each_Column_Info.Column_ID_Kind = eKind;
		Each_Column_Info.Column_ID_Kind_String = UOLEDB_Result::ColumnIdKindToString(eKind).Len();

        switch (eKind)
        {
            case DBKIND_GUID_NAME:
            {
                const FString GuidStr = UOLEDB_Result::GuidToString(Each_Column.columnid.uGuid.guid);
			    const FString uName = Each_Column.columnid.uName.pwszName ? FString(Each_Column.columnid.uName.pwszName) : FString();
			    Each_Column_Info.Column_ID_String = FString(GuidStr) + TEXT(" / ") + uName;
            
                break;
            }

		    case DBKIND_GUID_PROPID:
            {
                const FString GuidStr = UOLEDB_Result::GuidToString(Each_Column.columnid.uGuid.guid);
			    const FString ulPropid = FString::FromInt((int32)Each_Column.columnid.uName.ulPropid);
			    Each_Column_Info.Column_ID_String = FString(GuidStr) + TEXT(" / ") + ulPropid;

			    break;
            }

            case DBKIND_NAME:
                Each_Column_Info.Column_ID_String = Each_Column.columnid.uName.pwszName ? FString(Each_Column.columnid.uName.pwszName) : FString();
                break;

            case DBKIND_PGUID_NAME:
            {
                const GUID* pguid = Each_Column.columnid.uGuid.pguid;
                const FString GuidStr = pguid ? UOLEDB_Result::GuidToString(*pguid) : TEXT("<null-pguid>");
			    const FString uName = Each_Column.columnid.uName.pwszName ? FString(Each_Column.columnid.uName.pwszName) : FString();
			    Each_Column_Info.Column_ID_String = FString(GuidStr) + TEXT(" / ") + uName;

			    break;
            }

            case DBKIND_PGUID_PROPID:
		    {
				const GUID* pguid = Each_Column.columnid.uGuid.pguid;
			    const FString GuidStr = pguid ? UOLEDB_Result::GuidToString(*pguid) : TEXT("<null-pguid>");
			    const FString ulPropid = FString::FromInt((int32)Each_Column.columnid.uName.ulPropid);
			    Each_Column_Info.Column_ID_String = FString(GuidStr) + TEXT(" / ") + ulPropid;
                break;
		    }

            case DBKIND_PROPID:
			    Each_Column_Info.Column_ID_String = FString::FromInt((int32)Each_Column.columnid.uName.ulPropid);
			    break;

            case DBKIND_GUID:
			    Each_Column_Info.Column_ID_String = UOLEDB_Result::GuidToString(Each_Column.columnid.uGuid.guid);
                break;
        }

        Array_Columns_Infos.Add(MoveTemp(Each_Column_Info));
    }

    // Free OLE DB allocations
    CoTaskMemFree(Columns);
    CoTaskMemFree(pStringsBuffer);

	OutColumnInfo = MoveTemp(Array_Columns_Infos);
    return true;
}

bool UOLEDB_Result::GetColumnData(TArray<FString>& OutData, int32 ColumnIndex)
{
    if (!this->RowSetBuffer)
    {
        return false;
    }

    IRowset* pRowset = reinterpret_cast<IRowset*>(RowSetBuffer);

    if (!pRowset)
    {
        return false;
    }

    if (!this->bCursorAtStart)
    {
        this->ResetCursor();
    }

    // Get column info to validate the column index
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

    // Check if the column index is valid
    if (ColumnIndex < 0 || ColumnIndex >= (int32)cCols)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid column index: %d (column count: %d)"), ColumnIndex, cCols);
        CoTaskMemFree(Columns);
        CoTaskMemFree(pStringsBuffer);
        return false;
    }

    // Create accessor for the specified column
    DBCOLUMNINFO& ColumnInfo = Columns[ColumnIndex];
    HACCESSOR hAccessor = DB_NULL_HACCESSOR;
    DBBINDING Binding = { 0 };

    // Set up binding for the column
    Binding.iOrdinal = ColumnInfo.iOrdinal;
    Binding.dwPart = DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH;
    Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
    Binding.eParamIO = DBPARAMIO_NOTPARAM;
    Binding.cbMaxLen = ColumnInfo.ulColumnSize;
    Binding.wType = ColumnInfo.wType;
    Binding.obStatus = 0;
    Binding.obLength = sizeof(DBSTATUS);
    Binding.obValue = sizeof(DBSTATUS) + sizeof(DBLENGTH);

    // Get accessor from rowset
    IAccessor* pAccessor = nullptr;
    Result = pRowset->QueryInterface(IID_IAccessor, (void**)&pAccessor);

    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("QueryInterface(IAccessor) failed: 0x%08X"), Result);
        CoTaskMemFree(Columns);
        CoTaskMemFree(pStringsBuffer);
        return false;
    }

    Result = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &Binding, 0, &hAccessor, NULL);

    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("CreateAccessor failed: 0x%08X"), Result);
        pAccessor->Release();
        CoTaskMemFree(Columns);
        CoTaskMemFree(pStringsBuffer);
        return false;
    }

    // Prepare for fetching rows
    HROW* phRow = new HROW[1];
    DBCOUNTITEM cRowsObtained = 0;
    BYTE* pData = new BYTE[Binding.cbMaxLen + sizeof(DBSTATUS) + sizeof(DBLENGTH)];

    // Clear the output array
    OutData.Empty();

    // Fetch rows
    while (true)
    {
        Result = pRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow);

        if (FAILED(Result) || cRowsObtained == 0)
        {
            break;
        }

        // Initialize the buffer to zero
        FMemory::Memzero(pData, Binding.cbMaxLen + sizeof(DBSTATUS) + sizeof(DBLENGTH));

        // Get data for the row
        Result = pRowset->GetData(phRow[0], hAccessor, pData);

        if (SUCCEEDED(Result))
        {
            DBSTATUS* pStatus = (DBSTATUS*)pData;
            DBLENGTH* pLength = (DBLENGTH*)(pData + sizeof(DBSTATUS));
            void* pValue = (void*)(pData + sizeof(DBSTATUS) + sizeof(DBLENGTH));

            if (*pStatus == DBSTATUS_S_OK)
            {
                FString Value;

                // Convert the data to FString based on the column type
                switch (Binding.wType)
                {
                    case DBTYPE_WSTR:
                    {
                        // Handle UTF-16 encoded wide strings (OLEDB's native format)
                        WCHAR* wideStr = (WCHAR*)pValue;

                        // Get the actual length in characters (not including null terminator)
                        int32 Length = (*pLength) / sizeof(WCHAR);
                        
                        if (Length > 0)
                        {
							Value.AppendChars(wideStr, Length);
                        }

                        else
                        {
                            Value = TEXT("");
                        }

                        break;
                    }

                    case DBTYPE_STR:
                    {
                        // For ANSI strings, assume they're in UTF-8 encoding
                        int32 Length = (*pLength);
                        
                        if (Length > 0)
                        {
							Value = UOLEDB_Result::AnsiToFString((const char*)pValue);
                        }

                        else
                        {
                            Value = TEXT("");
                        }

                        break;
                    }

                    case DBTYPE_I2:
                        Value = FString::FromInt(*(int16*)pValue);
                        break;
                    case DBTYPE_I4:
                        Value = FString::FromInt(*(int32*)pValue);
                        break;
                    case DBTYPE_R4:
                        Value = FString::SanitizeFloat(*(float*)pValue);
                        break;
                    case DBTYPE_R8:
                        Value = FString::SanitizeFloat(*(double*)pValue);
                        break;
                    case DBTYPE_BOOL:
                        Value = *(VARIANT_BOOL*)pValue ? TEXT("True") : TEXT("False");
                        break;
                    case DBTYPE_GUID:
                        Value = GuidToString(*(GUID*)pValue);
                        break;
                    default:
                        Value = TEXT("[Unsupported type]");
                        break;
                }

                OutData.Add(Value);
            }
            else if (*pStatus == DBSTATUS_S_ISNULL)
            {
                OutData.Add(TEXT("NULL"));
            }
            else
            {
                OutData.Add(FString::Printf(TEXT("[Error: %d]"), *pStatus));
            }
        }

        // Release the row handle
        pRowset->ReleaseRows(1, phRow, NULL, NULL, NULL);
    }

    // Clean up
    pAccessor->ReleaseAccessor(hAccessor, NULL);
    pAccessor->Release();
    delete[] pData;
    delete[] phRow;
    CoTaskMemFree(Columns);
    CoTaskMemFree(pStringsBuffer);

	this->bCursorAtStart = false;
    return true;
}