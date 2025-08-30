#include "ODBC/ODBC_Tools.h"

FODBC_QueryHandler::~FODBC_QueryHandler()
{
    if (this->SQL_Handle)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, this->SQL_Handle);
        this->SQL_Handle = nullptr;
    }
}

bool FODBC_QueryHandler::SetSQLHandle(SQLHSTMT In_SQL_Handle)
{
    if (!In_SQL_Handle)
    {
        return false;
    }

	this->SQL_Handle = In_SQL_Handle;
    return true;
}

bool FODBC_QueryHandler::GetEachColumnInfo(FODBC_ColumnInfo& Out_MetaData, int32 ColumnIndex)
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

    FODBC_ColumnInfo EachMetaData;
    EachMetaData.Column_Name = UTF8_TO_TCHAR((const char*)Column_Name);
    EachMetaData.NameLenght = NameLen;
    EachMetaData.DataType = DataType;
    EachMetaData.DecimalDigits = DecimalDigits;
    EachMetaData.bIsNullable = Nullable == 1 ? true : false;
    EachMetaData.Column_Size = Column_Size;

    Out_MetaData = EachMetaData;

    return true;
}

FString FODBC_QueryHandler::GetChunckData(int32 SQL_Column_Index)
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

bool FODBC_QueryHandler::Record_Result(FString& Out_Code)
{
    if (!this->SQL_Handle)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    SQLRETURN RetCode = 0;
	TArray<FODBC_ResultSet> Pool_Result_Sets;

    try
    {
        do
        {
            FODBC_ResultSet Each_Result_Set;

            int32 Index_Row = 0;
            TArray<FODBC_ColumnInfo> Temp_Column_Infos;
            TMap<FVector2D, FODBC_DataValue> Temp_Data_Pool;

            // Update only queries like INSERT, UPDATE, DELETE, etc will return "Temp_AffectedRows" as -1.
            SQLLEN Temp_AffectedRows;
            RetCode = SQLRowCount(this->SQL_Handle, &Temp_AffectedRows);

            if (!SQL_SUCCEEDED(RetCode))
            {
                continue;
            }

            SQLSMALLINT Temp_ColumnNumber;
            RetCode = SQLNumResultCols(this->SQL_Handle, &Temp_ColumnNumber);

            if (!SQL_SUCCEEDED(RetCode))
            {
                continue;
            }

            while (SQLFetch(this->SQL_Handle) == SQL_SUCCESS)
            {
                for (int32 Column_Index = 0; Column_Index < Temp_ColumnNumber; Column_Index++)
                {
                    const int32 SQL_Column_Index = Column_Index + 1;

                    if (Temp_Column_Infos.Num() != Temp_ColumnNumber)
                    {
                        FODBC_ColumnInfo EachMetaData;
                        if (this->GetEachColumnInfo(EachMetaData, SQL_Column_Index))
                        {
                            Temp_Column_Infos.Add(EachMetaData);
                        }
                    }

                    FODBC_DataValue EachData;

                    if (Temp_Column_Infos.IsValidIndex(Column_Index))
                    {
                        EachData.ColumnName = Temp_Column_Infos[Column_Index].Column_Name;
                        EachData.DataType = Temp_Column_Infos[Column_Index].DataType;
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

                    const FVector2D Position = FVector2D(Column_Index, Index_Row);
                    Temp_Data_Pool.Add(Position, EachData);
                }

                Index_Row += 1;
            }

            Each_Result_Set.Data_Pool = Temp_Data_Pool;
            Each_Result_Set.Count_Rows = Index_Row;
            Each_Result_Set.Count_Columns = Temp_ColumnNumber;
            Each_Result_Set.Affected_Rows = Temp_AffectedRows < 0 ? 0 : Temp_AffectedRows;
			Each_Result_Set.Column_Infos = Temp_Column_Infos;

            Pool_Result_Sets.Add(Each_Result_Set);

        } while (SQL_SUCCEEDED(SQLMoreResults(this->SQL_Handle)));

		SQLFreeHandle(SQL_HANDLE_STMT, this->SQL_Handle);
		this->SQL_Handle = nullptr;

        this->ResultSet = Pool_Result_Sets.Num() > 0 ? Pool_Result_Sets.Last() : FODBC_ResultSet();
        Out_Code = "FF Microsoft ODBC : Result recording finished. Please check the result.";
        return true;
    }

    catch (const std::exception& Exception)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, this->SQL_Handle);
        this->SQL_Handle = nullptr;

        Out_Code = Exception.what();
        return false;
    }
}