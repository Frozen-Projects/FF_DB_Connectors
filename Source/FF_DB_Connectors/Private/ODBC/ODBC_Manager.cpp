#include "ODBC/ODBC_Manager.h"

// Sets default values.
AODBC_Manager::AODBC_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AODBC_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game ends or when destroyed.
void AODBC_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	this->Disconnect();
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AODBC_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

SQLHDBC AODBC_Manager::GetConnectionHandle()
{
	return this->ODBC_Connection;
}

FCriticalSection* AODBC_Manager::GetGuard()
{
	return &this->DB_Guard;
}

FString AODBC_Manager::GetConnectionString()
{
	return this->ConnectionString;
}

bool AODBC_Manager::CreateConnectionString(FString& Out_ConStr, FString ODBC_Source, FString Username, FString Password, FString ServerInstance)
{
	if (ODBC_Source.IsEmpty())
	{
		Out_ConStr = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : A Target server shouldn't be empty !";
		return false;
	}

	if (Username.IsEmpty())
	{
		Out_ConStr = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Username shouldn't be empty !";
		return false;
	}

	if (ServerInstance.IsEmpty())
	{
		Out_ConStr = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Server instance shouldn't be empty !";
		return false;
	}

	Out_ConStr = "{SQL Server};SERVER=" + ODBC_Source + "\\" + ServerInstance + ";DSN=" + ODBC_Source + ";UID=" + Username + ";PWD=" + Password;
	return true;
}

bool AODBC_Manager::ConnectDatabase(FString& Out_Code, const FString& In_ConStr)
{
	SQLRETURN RetCode = SQLAllocEnv(&this->ODBC_Environment);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Failed to allocate SQL environment";
		return false;
	}

	RetCode = SQLSetEnvAttr(this->ODBC_Environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Failed to set the ODBC version.";
		SQLFreeHandle(SQL_HANDLE_ENV, this->ODBC_Environment);
		return false;
	}

	RetCode = SQLAllocConnect(this->ODBC_Environment, &this->ODBC_Connection);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Failed to allocate a connection handle.";
		SQLFreeHandle(SQL_HANDLE_ENV, this->ODBC_Environment);
		return false;
	}

	RetCode = SQLDriverConnectA(this->ODBC_Connection, NULL, (SQLCHAR*)TCHAR_TO_UTF8(*In_ConStr), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
	if (!SQL_SUCCEEDED(RetCode))
	{
		SQLFreeHandle(SQL_HANDLE_DBC, this->ODBC_Connection);
		SQLFreeHandle(SQL_HANDLE_ENV, this->ODBC_Environment);

		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Connection couldn't made !";
		return false;
	}

	this->ConnectionString = In_ConStr;
	Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Connection successfully established !";
	return true;
}

void AODBC_Manager::CreateConnection(FDelegate_ODBC_Connection DelegateConnection, const FString& In_ConStr)
{
	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, In_ConStr]()
		{
			FString Out_Code;
			const bool ConnectionResult = this->ConnectDatabase(Out_Code, In_ConStr);

			AsyncTask(ENamedThreads::GameThread, [DelegateConnection, ConnectionResult, Out_Code]()
				{
					DelegateConnection.ExecuteIfBound(ConnectionResult, Out_Code);
				}
			);
		}
	);
}

void AODBC_Manager::Disconnect()
{
	if (this->ODBC_Connection)
	{
		SQLDisconnect(this->ODBC_Connection);
		SQLFreeHandle(SQL_HANDLE_DBC, this->ODBC_Connection);
		this->ODBC_Connection = NULL;
	}

	if (this->ODBC_Environment)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, this->ODBC_Environment);
		this->ODBC_Environment = NULL;
	}
}

SQLHSTMT AODBC_Manager::ExecuteQuery_Internal(FString& Out_Code, SQLHDBC In_Connection, FCriticalSection* In_Guard, const FString& SQL_Query)
{
	FScopeLock Lock(In_Guard);

	if (SQL_Query.IsEmpty())
	{
		return nullptr;
	}

	if (!In_Connection)
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Connection handle is not valid !";
		return 0;
	}

	SQLHSTMT SQL_Handle = SQL_NULL_HSTMT;
	SQLRETURN RetCode = SQLAllocHandle(SQL_HANDLE_STMT, In_Connection, &SQL_Handle);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : There was a problem while allocating statement handle : " + FString::FromInt(RetCode);
		return nullptr;
	}

	SQLWCHAR* StatementString = (SQLWCHAR*)(*SQL_Query);
	RetCode = SQLPrepareW(SQL_Handle, StatementString, SQL_NTS);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : There was a problem while preparing statement : " + FString::FromInt(RetCode);
		return nullptr;
	}

	RetCode = SQLSetStmtAttr(SQL_Handle, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_ON, 0);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : There was a problem while setting statement attribute : " + FString::FromInt(RetCode);
		return nullptr;
	}

	RetCode = SQLExecute(SQL_Handle);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : There was a problem while executing query : " + FString::FromInt(RetCode);
		return nullptr;
	}

	return SQL_Handle;
}

int32 AODBC_Manager::ExecuteQuery(FODBC_QueryHandler& Out_Handler, FString& Out_Code, SQLHDBC In_Connection, FCriticalSection* In_Guard, const FString& SQL_Query)
{
	SQLHSTMT ODBC_Statement = AODBC_Manager::ExecuteQuery_Internal(Out_Code, In_Connection, In_Guard, SQL_Query);

	if (!ODBC_Statement)
	{
		// Out_Code is already set in ExecuteQuery_Internal function.
		Out_Handler = FODBC_QueryHandler();
		return 0;
	}


	FODBC_QueryHandler Temp_Handler;
	const bool bResult = Temp_Handler.Record_Result(Out_Code, In_Connection, ODBC_Statement);
	
	SQLFreeHandle(SQL_HANDLE_STMT, ODBC_Statement);
	ODBC_Statement = nullptr;

	if (!bResult)
	{
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : There was a problem while setting connection handle to query handler !";
		Out_Handler = FODBC_QueryHandler();
		return 0;
	}

	else if (Temp_Handler.ResultSet.Count_Columns > 0)
	{
		Out_Handler = Temp_Handler;
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Query executed successfully !";
		return 1;
	}

	else if (Temp_Handler.ResultSet.Affected_Rows > 0)
	{
		Out_Handler = Temp_Handler;
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Query executed successfully but it is update only !";
		return 2;
	}

	else
	{
		Out_Handler = FODBC_QueryHandler();
		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Query executed successfully but there is no result or affected columns !";
		return 0;
	}
}

void AODBC_Manager::ExecuteQueryBp(FDelegate_ODBC_Execute DelegateExecute, const FString& SQL_Query)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, DelegateExecute, SQL_Query]()
		{
			FString Out_Code;
			FODBC_QueryHandler Temp_Handler;
			const int32 ExecuteResult = AODBC_Manager::ExecuteQuery(Temp_Handler, Out_Code, this->ODBC_Connection, &this->DB_Guard, SQL_Query);

			AsyncTask(ENamedThreads::GameThread, [this, DelegateExecute, Out_Code, Temp_Handler, ExecuteResult]()
				{
					if (ExecuteResult == 1)
					{
						UODBC_Result* ResultObject = NewObject<UODBC_Result>();
						ResultObject->SetQueryResult(Temp_Handler);

						DelegateExecute.ExecuteIfBound(ExecuteResult, Out_Code, ResultObject, Temp_Handler.ResultSet.Affected_Rows);
					}

					else
					{
						DelegateExecute.ExecuteIfBound(ExecuteResult, Out_Code, nullptr, Temp_Handler.ResultSet.Affected_Rows);
					}
				}
			);
		}
	);
}

bool AODBC_Manager::LearnColumns_Internal(TArray<FODBC_ColumnInfo>& Out_ColumnInfos, FString& Out_Code, const FString& SQL_Query)
{
	SQLHSTMT ODBC_Statement = AODBC_Manager::ExecuteQuery_Internal(Out_Code, this->ODBC_Connection, &this->DB_Guard, SQL_Query);

	if (!ODBC_Statement)
	{
		return false;
	}

	SQLSMALLINT Temp_Count_Column = 0;
	SQLRETURN RetCode = SQLNumResultCols(ODBC_Statement, &Temp_Count_Column);

	if (Temp_Count_Column == 0)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, ODBC_Statement);
		ODBC_Statement = nullptr;

		Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : There is no column to get metadata !";
		return false;
	}

	const int32 MaxColNameLen = ODBC_UtilityClass::MaxColumnNameLength(this->ODBC_Connection);

	TArray<FODBC_ColumnInfo> Pool_CI;

	for (size_t ColumnIndex = 0; ColumnIndex < Temp_Count_Column; ColumnIndex++)
	{
		// SQL columns start from 1, not 0.
		const size_t SQL_Column_Index = ColumnIndex + 1;

		TUniquePtr<SQLWCHAR[]> Column_NamePtr = MakeUnique<SQLWCHAR[]>(MaxColNameLen);
		SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
		SQLULEN Column_Size;

		RetCode = SQLDescribeColW(ODBC_Statement, SQL_Column_Index, Column_NamePtr.Get(), MaxColNameLen, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

		if (!SQL_SUCCEEDED(RetCode))
		{
			continue;
		}

		const UTF16CHAR* Utf16Ptr = reinterpret_cast<const UTF16CHAR*>(Column_NamePtr.Get());
		const FTCHARToWChar Converter = StringCast<TCHAR>(Utf16Ptr, NameLen);
		FString ColumnName(Converter.Length(), Converter.Get());

		FODBC_ColumnInfo Each_Column;
		Each_Column.Column_Name = ColumnName;
		Each_Column.NameLenght = NameLen;
		Each_Column.DataType = DataType;
		Each_Column.DecimalDigits = DecimalDigits;
		Each_Column.bIsNullable = Nullable == 1 ? true : false;
		Each_Column.Column_Size = Column_Size;

		switch (DataType)
		{
			// NVARCHAR & DATE & TIME
			case -9:
			{
				Each_Column.DataTypeName = "NVARCHAR & DATE & TIME";
				break;
			}

			// INT64 & BIGINT
			case -5:
			{
				Each_Column.DataTypeName = "INT64 & BIGINT";
				break;
			}

			// TIMESTAMP
			case -2:
			{
				Each_Column.DataTypeName = "TIMESTAMP";
				break;
			}

			// TEXT
			case -1:
			{
				Each_Column.DataTypeName = "TEXT";
				break;
			}

			// INT32
			case 4:
			{
				Each_Column.DataTypeName = "INT32";
				break;
			}

			// FLOAT & DOUBLE
			case 6:
			{
				Each_Column.DataTypeName = "FLOAT & DOUBLE";
				break;
			}

			// DATETIME
			case 93:
			{
				Each_Column.DataTypeName = "DATETIME";
				break;
			}

			default:
			{
				Each_Column.DataTypeName = "UNKNOWN";
				break;
			}
		}

		Pool_CI.Add(Each_Column);
	}

	SQLFreeHandle(SQL_HANDLE_STMT, ODBC_Statement);
	ODBC_Statement = nullptr;

	Out_Code = "FF_DB_Connectors : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Column infos extracted successfully !";
	Out_ColumnInfos = Pool_CI;
	return true;
}

void AODBC_Manager::LearnColumnsBp(FDelegate_ODBC_CI DelegateColumnInfos, const FString& SQL_Query)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, DelegateColumnInfos, SQL_Query]()
		{
			FString Out_Code;
			TArray<FODBC_ColumnInfo> ColumnInfos;
			const bool bResult = this->LearnColumns_Internal(ColumnInfos, Out_Code, SQL_Query);
			
			AsyncTask(ENamedThreads::GameThread, [DelegateColumnInfos, bResult, Out_Code, ColumnInfos]()
				{
					DelegateColumnInfos.ExecuteIfBound(bResult, Out_Code, bResult ? ColumnInfos : TArray<FODBC_ColumnInfo>());
				}
			);
		}
	);
}