#include "ODBC/MS_ODBC_Result.h"

FString UODBC_Result::AnsiToFString(const char* AnsiStr)
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

bool UODBC_Result::SetQueryResult(const SQLHSTMT& In_Handle, const FString& In_Query)
{
    if (!In_Handle)
    {
        return false;
    }

    SQLLEN AffectedRows;
    SQLRowCount(In_Handle, &AffectedRows);

    SQLSMALLINT ColumnNumber;
    SQLNumResultCols(In_Handle, &ColumnNumber);

    this->Affected_Rows = AffectedRows;
    this->Count_Column = ColumnNumber;
    this->SQL_Handle_Statement = In_Handle;
    this->SentQuery = In_Query;

    return true;
}

bool UODBC_Result::GetEachMetaData(FMS_ODBC_MetaData& Out_MetaData, int32 ColumnIndex)
{
    if (!this->SQL_Handle_Statement)
    {
        return false;
    }

    if (this->Count_Column == 0)
    {
        return false;
    }

    const int32 Column_Name_Size = 256;
    SQLCHAR Column_Name[Column_Name_Size];
    SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
    SQLULEN Column_Size;

    SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle_Statement, ColumnIndex, Column_Name, Column_Name_Size, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

    if (!SQL_SUCCEEDED(RetCode))
    {
        return false;
    }

    FString Column_Name_String = UTF8_TO_TCHAR((const char*)Column_Name);
    Column_Name_String.TrimEndInline();

    FMS_ODBC_MetaData EachMetaData;
    EachMetaData.Column_Name = Column_Name_String;
    EachMetaData.NameLenght = NameLen;
    EachMetaData.DataType = DataType;
    EachMetaData.DecimalDigits = DecimalDigits;
    EachMetaData.bIsNullable = Nullable == 1 ? true : false;
    EachMetaData.Column_Size = Column_Size;

    Out_MetaData = EachMetaData;

    return true;
}

// BLUEPRINT EXPOSED

bool UODBC_Result::Result_Record(FString& Out_Code)
{
    if (this->bIsResultRecorded)
    {
        Out_Code = "FF Microsoft ODBC : Result already recorded.";
        return false;
    }

    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    if (this->Count_Column == 0)
    {
        Out_Code = "FF Microsoft ODBC : This query doesn't have result. It is for update only !";
        return false;
    }

    try
    {
        TMap<FVector2D, FMS_ODBC_DataValue> Temp_Data_Pool;
        int32 Index_Row = 0;
        bool bIsMetaDataCollected = false;
        TArray<FMS_ODBC_MetaData> Array_MetaData;

        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
            {
                const FVector2D Position = FVector2D(Index_Column_Raw, Index_Row);
                
				// ODBC column index starts from 1.
                const int32 Index_Column = Index_Column_Raw + 1;

                if (!bIsMetaDataCollected)
                {
                    FMS_ODBC_MetaData EachMetaData;
                    if (this->GetEachMetaData(EachMetaData, Index_Column))
                    {
                        Array_MetaData.Add(EachMetaData);
                    }
                }

				const FString Column_Name = Array_MetaData[Index_Column_Raw].Column_Name;
				const int32 DataType = Array_MetaData[Index_Column_Raw].DataType;

                SQLLEN PreviewLenght;
                SQLCHAR PreviewData[SQL_MAX_TEXT_LENGHT];
                SQLRETURN RetCode = SQLGetData(this->SQL_Handle_Statement, Index_Column, SQL_CHAR, PreviewData, SQL_MAX_TEXT_LENGHT, &PreviewLenght);

                if (!SQL_SUCCEEDED(RetCode))
                {
                    Out_Code = "FF Microsoft ODBC : There was a problem while getting SQL Data : " + Position.ToString();
                    return false;
                }

				std::stringstream Stream;
                Stream.write((char*)&PreviewData, PreviewLenght);
				const std::string RawString = Stream.str();

                const FString PreviewString = UODBC_Result::AnsiToFString(RawString.c_str());

                FMS_ODBC_DataValue EachData;
                EachData.ColumnName = Column_Name;
                EachData.DataType = DataType;
                EachData.Preview = PreviewString;

                Temp_Data_Pool.Add(Position, EachData);
            }

            bIsMetaDataCollected = true;
            Index_Row += 1;
        }

        this->Data_Pool = Temp_Data_Pool;
        this->Count_Row = Index_Row;
        this->bIsResultRecorded = true;

        Out_Code = "FF Microsoft ODBC : Result successfuly recorded !";
        return true;
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }
}

void UODBC_Result::Result_Record_Async(FDelegate_MS_ODBC_Record DelegateRecord)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateRecord]()
        {
            FString Out_Code;

            if (this->Result_Record(Out_Code))
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateRecord, Out_Code]()
                    {
                        DelegateRecord.ExecuteIfBound(true, Out_Code);
                    }
                );

                return;
            }

            else
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateRecord, Out_Code]()
                    {
                        DelegateRecord.ExecuteIfBound(false, Out_Code);
                    }
                );

                return;
            }
        }
    );
}

bool UODBC_Result::Result_Fetch(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnIndex)
{
    if (ColumnIndex < 1)
    {
        Out_Code = "FF Microsoft ODBC : Column index starts from 1 !";
        return false;
    }

    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    try
    {
        TArray<FString> Array_Temp;
        int32 Index_Row = 0;

        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            SQLLEN ReceivedLenght;
            SQLCHAR TempData[SQL_MAX_TEXT_LENGHT];
            SQLGetData(this->SQL_Handle_Statement, ColumnIndex, SQL_CHAR, TempData, SQL_MAX_TEXT_LENGHT, &ReceivedLenght);

            FString EachData;
            EachData.AppendChars((const char*)TempData, ReceivedLenght);
            EachData.TrimEndInline();
            Array_Temp.Add(EachData);

            Index_Row += 1;
        }

        this->Count_Row = Index_Row;
        Out_Values = Array_Temp;
        Out_Code = "FF Microsoft ODBC : Result successfully fetched !";

        return true;
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }
}

void UODBC_Result::Result_Fetch_Async(FDelegate_MS_ODBC_Fetch DelegateFetch, int32 ColumnIndex)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateFetch, ColumnIndex]()
        {
            FString Out_Code;
            TArray<FString> Out_Values;

            if (this->Result_Fetch(Out_Code, Out_Values, ColumnIndex))
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateFetch, Out_Code, Out_Values]()
                    {
                        DelegateFetch.ExecuteIfBound(true, Out_Code, Out_Values);
                    }
                );

                return;
            }

            else
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateFetch, Out_Code]()
                    {
                        DelegateFetch.ExecuteIfBound(false, Out_Code, TArray<FString>());
                    }
                );

                return;
            }
        }
    );
}

int32 UODBC_Result::GetColumnNumber()
{
    return this->Count_Column;
}

int32 UODBC_Result::GetRowNumber()
{
    return this->Count_Row;
}

int32 UODBC_Result::GetAffectedRows()
{
    return this->Affected_Rows;
}

bool UODBC_Result::GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Row)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Row < 0 || Index_Row >= this->Count_Row)
    {
        Out_Code = "FF Microsoft ODBC : Given row index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < this->Count_Column; Index_Column++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FMS_ODBC_DataValue* EachData = this->Data_Pool.Find(Position);

        if (!EachData)
        {
            Out_Code = "FF Microsoft ODBC : Found data is not valid : " + Position.ToString();
            return false;
        }

        Temp_Array.Add(*EachData);
    }

    Out_Code = "FF Microsoft ODBC : Row exported successfully !";
    Out_Values = Temp_Array;
    return true;
}

bool UODBC_Result::GetColumnFromIndex(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Column)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Column < 0 || Index_Column >= this->Count_Column)
    {
        Out_Code = "FF Microsoft ODBC : Given column index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Row = 0; Index_Row < this->Count_Row; Index_Row++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FMS_ODBC_DataValue* EachData = this->Data_Pool.Find(Position);

        if (!EachData)
        {
            Out_Code = "FF Microsoft ODBC : Found data is not valid : " + Position.ToString();
            return false;
        }

        Temp_Array.Add(*EachData);
    }

    Out_Code = "FF Microsoft ODBC : Column exported successfully !";
    Out_Values = Temp_Array;
    return true;
}

bool UODBC_Result::GetColumnFromName(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, FString ColumName)
{
    if (this->Count_Column == 0)
    {
        return false;
    }

    int32 TargetIndex = 0;
    for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
    {
        const int32 Index_Column = Index_Column_Raw + 1;
        SQLCHAR Column_Name[256];
        SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
        SQLULEN Column_Size;

        SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle_Statement, Index_Column, Column_Name, 256, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

        if (!SQL_SUCCEEDED(RetCode))
        {
            Out_Code = "FF Microsoft ODBC : There was a problem while getting index of target column !";
            return false;
        }

        FString EachName = UTF8_TO_TCHAR((const char*)Column_Name);
        EachName.TrimEndInline();

        if (EachName == ColumName)
        {
            TargetIndex = Index_Column_Raw;
            break;
        }
    }

    return this->GetColumnFromIndex(Out_Code, Out_Values, TargetIndex);
}

bool UODBC_Result::GetSingleData(FString& Out_Code, FMS_ODBC_DataValue& Out_Value, FVector2D Position)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Position.X < 0 || Position.Y < 0 || Position.X >= this->Count_Column || Position.Y >= this->Count_Row)
    {
        Out_Code = "FF Microsoft ODBC : Given position is out of data pool's range !";
        return false;
    }

    if (!this->Data_Pool.Contains(Position))
    {
        Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
        return false;
    }

    FMS_ODBC_DataValue* DataValue = this->Data_Pool.Find(Position);

    if (!DataValue)
    {
        Out_Code = "FF Microsoft ODBC : Found data is not valid !";
        return false;
    }

    Out_Value = *DataValue;
    return true;
}

bool UODBC_Result::GetMetaData(FString& Out_Code, TArray<FMS_ODBC_MetaData>& Out_MetaData)
{
    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    SQLSMALLINT Temp_Count_Column = 0;
    SQLRETURN RetCode = SQLNumResultCols(this->SQL_Handle_Statement, &Temp_Count_Column);

    if (Temp_Count_Column == 0)
    {
        Out_Code = "FF Microsoft ODBC : There is no column to get metadata !";
        return false;
    }

    TArray<FMS_ODBC_MetaData> Array_MetaData;

    for (int32 Index_Column_Raw = 0; Index_Column_Raw < Temp_Count_Column; Index_Column_Raw++)
    {
        FMS_ODBC_MetaData EachMetaData;
        if (this->GetEachMetaData(EachMetaData, Index_Column_Raw + 1))
        {
            Array_MetaData.Add(EachMetaData);
        }
    }

    Out_MetaData = Array_MetaData;
    Out_Code = "FF Microsoft ODBC : All metadata got successfully !";
    return true;
}