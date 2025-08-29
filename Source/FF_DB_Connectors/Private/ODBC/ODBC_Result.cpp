#include "ODBC/ODBC_Result.h"

void UODBC_Result::BeginDestroy()
{
    if (this->QueryHandler.SQL_Handle)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, this->QueryHandler.SQL_Handle);
        this->QueryHandler.SQL_Handle = NULL;
	}

    Super::BeginDestroy();
}

bool UODBC_Result::SetQueryResult(FODBC_QueryHandler In_Handler)
{
    if (!In_Handler.SQL_Handle)
    {
        return false;
    }

	FScopeLock Lock(&this->ResultGuard);
    this->QueryHandler = In_Handler;

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

bool UODBC_Result::GetColumnInfos(FString& Out_Code, TArray<FODBC_ColumnInfo>& Out_MetaData)
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

    TArray<FODBC_ColumnInfo> Array_MetaData;

    for (int32 Index_Column_Raw = 0; Index_Column_Raw < Temp_Count_Column; Index_Column_Raw++)
    {
        FODBC_ColumnInfo EachMetaData;
        if (this->QueryHandler.GetEachColumnInfo(EachMetaData, Index_Column_Raw + 1))
        {
            Array_MetaData.Add(EachMetaData);
        }
    }

    Out_MetaData = Array_MetaData;
    Out_Code = "FF Microsoft ODBC : All metadata got successfully !";
    return true;
}

bool UODBC_Result::GetColumnFromIndex(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, int64 Index_Column)
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

    TArray<FODBC_DataValue> Temp_Array;

    for (int32 Index_Row = 0; Index_Row < this->QueryHandler.Count_Rows; Index_Row++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->QueryHandler.Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FODBC_DataValue* EachData = this->QueryHandler.Data_Pool.Find(Position);

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

bool UODBC_Result::GetColumnFromName(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, FString ColumName)
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

bool UODBC_Result::GetSingleData(FString& Out_Code, FODBC_DataValue& Out_Value, FVector2D Position)
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

    FODBC_DataValue* DataValue = this->QueryHandler.Data_Pool.Find(Position);

    if (!DataValue)
    {
        Out_Code = "FF Microsoft ODBC : Found data is not valid !";
        return false;
    }

    Out_Value = *DataValue;
    return true;
}

bool UODBC_Result::GetRow(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, int64 Index_Row)
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

    TArray<FODBC_DataValue> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < this->QueryHandler.Count_Columns; Index_Column++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->QueryHandler.Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FODBC_DataValue* EachData = this->QueryHandler.Data_Pool.Find(Position);

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