#include "ODBC/ODBC_Result.h"

void UODBC_Result::BeginDestroy()
{
    Super::BeginDestroy();
}

bool UODBC_Result::SetQueryResult(FODBC_QueryHandler In_Handler)
{
	FScopeLock Lock(&this->ResultGuard);
    this->QueryHandler = In_Handler;

    return true;
}

int64 UODBC_Result::GetColumnNumber()
{
    return this->QueryHandler.ResultSet.Count_Columns;
}

int64 UODBC_Result::GetRowNumber()
{
    return this->QueryHandler.ResultSet.Count_Rows;
}

int64 UODBC_Result::GetAffectedRows()
{
    return this->QueryHandler.ResultSet.Affected_Rows;
}

FString UODBC_Result::GetQueryString()
{
    return this->QueryHandler.SentQuery;
}

bool UODBC_Result::GetColumnInfos(FString& Out_Code, TArray<FODBC_ColumnInfo>& Out_Infos)
{
    if (this->QueryHandler.ResultSet.Column_Infos.IsEmpty())
    {
		Out_Code = "FF Microsoft ODBC : There is no column info !";
        return false;
    }

	Out_Infos = this->QueryHandler.ResultSet.Column_Infos;
    Out_Code = "FF Microsoft ODBC : All metadata got successfully !";
    return true;
}

bool UODBC_Result::GetColumnFromIndex(FString& Out_Code, TArray<FODBC_DataValue>& Out_Values, int64 Index_Column)
{
    if (this->QueryHandler.ResultSet.Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Column < 0 || Index_Column >= this->QueryHandler.ResultSet.Count_Columns)
    {
        Out_Code = "FF Microsoft ODBC : Given column index is out of data pool's range !";
        return false;
    }

    TArray<FODBC_DataValue> Temp_Array;

    for (int32 Index_Row = 0; Index_Row < this->QueryHandler.ResultSet.Count_Rows; Index_Row++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->QueryHandler.ResultSet.Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FODBC_DataValue* EachData = this->QueryHandler.ResultSet.Data_Pool.Find(Position);

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
    if (this->QueryHandler.ResultSet.Count_Columns == 0)
    {
        return false;
    }

	int64 TargetIndex = -1;

    for (int64 Column_Index = 0; Column_Index < this->QueryHandler.ResultSet.Count_Columns; Column_Index++)
    {
		const FODBC_ColumnInfo EachColumnInfo = this->QueryHandler.ResultSet.Column_Infos[Column_Index];

        if (EachColumnInfo.Column_Name.Equals(ColumName))
        {
            TargetIndex = Column_Index;
        }
    }

    return this->GetColumnFromIndex(Out_Code, Out_Values, TargetIndex);
}

bool UODBC_Result::GetSingleData(FString& Out_Code, FODBC_DataValue& Out_Value, FVector2D Position)
{
    if (this->QueryHandler.ResultSet.Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Position.X < 0 || Position.Y < 0 || Position.X >= this->QueryHandler.ResultSet.Count_Columns || Position.Y >= this->QueryHandler.ResultSet.Count_Rows)
    {
        Out_Code = "FF Microsoft ODBC : Given position is out of data pool's range !";
        return false;
    }

    if (!this->QueryHandler.ResultSet.Data_Pool.Contains(Position))
    {
        Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
        return false;
    }

    FODBC_DataValue* DataValue = this->QueryHandler.ResultSet.Data_Pool.Find(Position);

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
    if (this->QueryHandler.ResultSet.Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Row < 0 || Index_Row >= this->QueryHandler.ResultSet.Count_Rows)
    {
        Out_Code = "FF Microsoft ODBC : Given row index is out of data pool's range !";
        return false;
    }

    TArray<FODBC_DataValue> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < this->QueryHandler.ResultSet.Count_Columns; Index_Column++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->QueryHandler.ResultSet.Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FODBC_DataValue* EachData = this->QueryHandler.ResultSet.Data_Pool.Find(Position);

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