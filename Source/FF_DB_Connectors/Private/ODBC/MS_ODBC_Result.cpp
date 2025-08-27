#include "ODBC/MS_ODBC_Result.h"

bool FMS_ODBC_QueryHandler::GetEachMetaData(FMS_ODBC_MetaData& Out_MetaData, int32 ColumnIndex)
{
    if (!this->SQL_Handle)
    {
        return false;
    }

    SQLSMALLINT Temp_ColumnNumber;
    SQLNumResultCols(SQL_Handle, &Temp_ColumnNumber);

    if (Temp_ColumnNumber == 0)
    {
        return false;
    }

    const int32 Column_Name_Size = 256;
    SQLCHAR Column_Name[Column_Name_Size];
    SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
    SQLULEN Column_Size;

    SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle, ColumnIndex, Column_Name, Column_Name_Size, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

    if (!SQL_SUCCEEDED(RetCode))
    {
        return false;
    }

    FMS_ODBC_MetaData EachMetaData;
    EachMetaData.Column_Name = UTF8_TO_TCHAR((const char*)Column_Name);
    EachMetaData.NameLenght = NameLen;
    EachMetaData.DataType = DataType;
    EachMetaData.DecimalDigits = DecimalDigits;
    EachMetaData.bIsNullable = Nullable == 1 ? true : false;
    EachMetaData.Column_Size = Column_Size;

    Out_MetaData = EachMetaData;

    return true;
}

FString FMS_ODBC_QueryHandler::GetChunckData(int32 SQL_Column_Index)
{
	FString Accumulated;

    constexpr SQLLEN WCHUNK = 4096;                         // wchar_t units requested per call
    constexpr SQLLEN BYTES = WCHUNK * sizeof(SQLWCHAR);     // buffer size in bytes
    SQLWCHAR Buffer[WCHUNK];                                // stack buffer per chunk

    SQLLEN Indicator = 0;

    while (true)
    {
        SQLRETURN Ret = SQLGetData(this->SQL_Handle, static_cast<SQLUSMALLINT>(SQL_Column_Index), SQL_C_WCHAR, Buffer, BYTES, &Indicator);

        if (Indicator == SQL_NULL_DATA)
        {
            // Column is NULL → leave Accumulated empty (or set a flag if you need)
            Accumulated.Reset();
            break;
        }

        if (!SQL_SUCCEEDED(Ret) && Ret != SQL_SUCCESS_WITH_INFO)
        {
            return FString();
        }

        // How many *bytes* are valid this call (exclude driver's trailing L'\0')
        size_t BytesThisCall =
            (Ret == SQL_SUCCESS_WITH_INFO) ? static_cast<size_t>(BYTES - sizeof(SQLWCHAR))
            : (Indicator == SQL_NO_TOTAL) ? static_cast<size_t>(BYTES - sizeof(SQLWCHAR))
            : (Indicator >= 0) ? static_cast<size_t>(FMath::Min<SQLLEN>(Indicator, BYTES - sizeof(SQLWCHAR)))
            : 0;

        const size_t WCharsThisCall = BytesThisCall / sizeof(SQLWCHAR);

        if (WCharsThisCall > 0)
        {
            // Append exactly the produced chars; FString is UTF-16 on Windows.
            Accumulated.AppendChars(reinterpret_cast<const TCHAR*>(Buffer), static_cast<int32>(WCharsThisCall));
        }

        // If not truncated, we’re done (no more chunks).
        if (Ret != SQL_SUCCESS_WITH_INFO)
        {
            break;
        }
    }

	return Accumulated;
}

int32 FMS_ODBC_QueryHandler::Record_Result(FString& Out_Code)
{
    if (!this->SQL_Handle)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return 0;
    }

    SQLLEN Temp_AffectedRows;
    SQLRowCount(this->SQL_Handle, &Temp_AffectedRows);

    SQLSMALLINT Temp_ColumnNumber;
    SQLNumResultCols(this->SQL_Handle, &Temp_ColumnNumber);

    if (Temp_AffectedRows == 0 || Temp_ColumnNumber == 0)
    {
		this->Affected_Rows = Temp_AffectedRows;
        Out_Code = "FF Microsoft ODBC : This query doesn't have result. It is for update only !";
        return 2;
    }

    try
    {
        int32 Index_Row = 0;
        TArray<FMS_ODBC_MetaData> Array_MetaData;
        TMap<FVector2D, FMS_ODBC_DataValue> Temp_Data_Pool;

        while (SQLFetch(this->SQL_Handle) == SQL_SUCCESS)
        {
            for (int32 Column_Index = 0; Column_Index < Temp_ColumnNumber; Column_Index++)
            {
                const int32 SQL_Column_Index = Column_Index + 1;

                if (Array_MetaData.IsEmpty())
                {
                    FMS_ODBC_MetaData EachMetaData;
                    if (this->GetEachMetaData(EachMetaData, SQL_Column_Index))
                    {
                        Array_MetaData.Add(EachMetaData);
                    }
                }

                const FVector2D Position = FVector2D(Column_Index, Index_Row);

                FMS_ODBC_DataValue EachData;

                if (Array_MetaData.IsValidIndex(Column_Index))
                {
                    EachData.ColumnName = Array_MetaData[Column_Index].Column_Name;
                    EachData.DataType = Array_MetaData[Column_Index].DataType;
                }

				FString Preview = GetChunckData(SQL_Column_Index);
                EachData.Preview = MoveTemp(Preview);

                switch (EachData.DataType)
                {
                    // NVARCHAR & DATE & TIME
                    case -9:
                    {
						EachData.DataTypeName = "NVARCHAR & DATE & TIME";
                        EachData.String = EachData.Preview;
                        break;
                    }

                    // INT64 & BIGINT
                    case -5:
                    {
						EachData.DataTypeName = "INT64 & BIGINT";
                        EachData.Integer64 = FCString::Atoi64(*EachData.Preview);
                        break;
                    }

                    // TIMESTAMP
                    case -2:
                    {
						EachData.DataTypeName = "TIMESTAMP";
                        std::string RawString = TCHAR_TO_UTF8(*EachData.Preview);
                        unsigned int TimeStampInt = std::stoul(RawString, nullptr, 16);

                        EachData.Integer64 = TimeStampInt;
                        EachData.Preview += " - " + FString::FromInt(TimeStampInt);
                        break;
                    }

                    // TEXT
                    case -1:
                    {
						EachData.DataTypeName = "TEXT";
                        EachData.String = EachData.Preview;
                        break;
                    }

                    // INT32
                    case 4:
                    {
						EachData.DataTypeName = "INT32";
                        EachData.Integer32 = FCString::Atoi(*EachData.Preview);
                        break;
                    }

                    // FLOAT & DOUBLE
                    case 6:
                    {
						EachData.DataTypeName = "FLOAT & DOUBLE";
                        EachData.Double = FCString::Atod(*EachData.Preview);
                        break;
                    }

                    // DATETIME
                    case 93:
                    {
						EachData.DataTypeName = "DATETIME";
                        TArray<FString> Array_Sections;
                        EachData.Preview.ParseIntoArray(Array_Sections, TEXT(" "));

                        FString Date = Array_Sections[0];
                        FString Time = Array_Sections[1];

                        TArray<FString> Array_Sections_Date;
                        Date.ParseIntoArray(Array_Sections_Date, TEXT("-"));
                        int32 Year = FCString::Atoi(*Array_Sections_Date[0]);
                        int32 Month = FCString::Atoi(*Array_Sections_Date[1]);
                        int32 Day = FCString::Atoi(*Array_Sections_Date[2]);

                        TArray<FString> Array_Sections_Time;
                        Time.ParseIntoArray(Array_Sections_Time, TEXT("."));
                        int32 Milliseconds = FCString::Atoi(*Array_Sections_Time[1]);
                        FString Clock = Array_Sections_Time[0];

                        TArray<FString> Array_Sections_Clock;
                        Clock.ParseIntoArray(Array_Sections_Clock, TEXT(":"));
                        int32 Hours = FCString::Atoi(*Array_Sections_Clock[0]);
                        int32 Minutes = FCString::Atoi(*Array_Sections_Clock[1]);
                        int32 Seconds = FCString::Atoi(*Array_Sections_Clock[2]);

                        EachData.DateTime = FDateTime(Year, Month, Day, Hours, Minutes, Seconds, Milliseconds);
                        break;
                    }

                    default:
                    {
                        EachData.Note = "Currently there is no parser for this data type. Please convert it to another known type in your query !";
                        break;
                    }
                }

                Temp_Data_Pool.Add(Position, EachData);
            }

            Index_Row += 1;
        }

        this->Data_Pool = Temp_Data_Pool;
        this->Count_Rows = Index_Row;
		this->Affected_Rows = Temp_AffectedRows;
		this->Count_Columns = Temp_ColumnNumber;

        Out_Code = "FF Microsoft ODBC : Result successfuly recorded !";
        return 1;
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return 0;
    }
}

#pragma region UODBC_Result

void UODBC_Result::BeginDestroy()
{
    if (this->QueryHandler.SQL_Handle)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, this->QueryHandler.SQL_Handle);
        this->QueryHandler.SQL_Handle = NULL;
	}

    Super::BeginDestroy();
}

bool UODBC_Result::SetQueryResult(FMS_ODBC_QueryHandler In_Handler)
{
    if (!In_Handler.SQL_Handle)
    {
        return false;
    }

	this->ResultGuard.Lock();
    this->QueryHandler = In_Handler;
	this->ResultGuard.Unlock();

    return true;
}

int64 UODBC_Result::GetColumnNumber()
{
    return this->QueryHandler.Count_Columns;
}

int64 UODBC_Result::GetRowNumber()
{
    return this->QueryHandler.Count_Rows;
}

int64 UODBC_Result::GetAffectedRows()
{
    return this->QueryHandler.Affected_Rows;
}

FString UODBC_Result::GetQueryString()
{
    return this->QueryHandler.SentQuery;
}

bool UODBC_Result::GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Row)
{
    if (this->QueryHandler.Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Row < 0 || Index_Row >= this->QueryHandler.Count_Rows)
    {
        Out_Code = "FF Microsoft ODBC : Given row index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < this->QueryHandler.Count_Columns; Index_Column++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->QueryHandler.Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FMS_ODBC_DataValue* EachData = this->QueryHandler.Data_Pool.Find(Position);

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
    if (this->QueryHandler.Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Column < 0 || Index_Column >= this->QueryHandler.Count_Columns)
    {
        Out_Code = "FF Microsoft ODBC : Given column index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Row = 0; Index_Row < this->QueryHandler.Count_Rows; Index_Row++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->QueryHandler.Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FMS_ODBC_DataValue* EachData = this->QueryHandler.Data_Pool.Find(Position);

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
    if (this->QueryHandler.Count_Columns == 0)
    {
        return false;
    }

    int32 TargetIndex = 0;
    for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->QueryHandler.Count_Columns; Index_Column_Raw++)
    {
        const int32 Index_Column = Index_Column_Raw + 1;
        SQLCHAR Column_Name[256];
        SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
        SQLULEN Column_Size;

        SQLRETURN RetCode = SQLDescribeColA(this->QueryHandler.SQL_Handle, Index_Column, Column_Name, 256, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

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
    if (this->QueryHandler.Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Position.X < 0 || Position.Y < 0 || Position.X >= this->QueryHandler.Count_Columns || Position.Y >= this->QueryHandler.Count_Rows)
    {
        Out_Code = "FF Microsoft ODBC : Given position is out of data pool's range !";
        return false;
    }

    if (!this->QueryHandler.Data_Pool.Contains(Position))
    {
        Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
        return false;
    }

    FMS_ODBC_DataValue* DataValue = this->QueryHandler.Data_Pool.Find(Position);

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
    if (!this->QueryHandler.SQL_Handle)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    SQLSMALLINT Temp_Count_Column = 0;
    SQLRETURN RetCode = SQLNumResultCols(this->QueryHandler.SQL_Handle, &Temp_Count_Column);

    if (Temp_Count_Column == 0)
    {
        Out_Code = "FF Microsoft ODBC : There is no column to get metadata !";
        return false;
    }

    TArray<FMS_ODBC_MetaData> Array_MetaData;

    for (int32 Index_Column_Raw = 0; Index_Column_Raw < Temp_Count_Column; Index_Column_Raw++)
    {
        FMS_ODBC_MetaData EachMetaData;
        if (this->QueryHandler.GetEachMetaData(EachMetaData, Index_Column_Raw + 1))
        {
            Array_MetaData.Add(EachMetaData);
        }
    }

    Out_MetaData = Array_MetaData;
    Out_Code = "FF Microsoft ODBC : All metadata got successfully !";
    return true;
}

#pragma endregion