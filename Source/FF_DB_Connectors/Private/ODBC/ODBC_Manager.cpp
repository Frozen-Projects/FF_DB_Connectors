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

bool AODBC_Manager::CreateConnectionString(FString& Out_ConStr, FString ODBC_Source, FString Username, FString Password, FString ServerInstance)
{
	if (ODBC_Source.IsEmpty())
	{
		Out_ConStr = "FF Microsoft ODBC : Target server shouldn't be empty !";
		return true;
	}

	if (Username.IsEmpty())
	{
		Out_ConStr = "FF Microsoft ODBC : Username shouldn't be empty !";
		return false;
	}

	if (ServerInstance.IsEmpty())
	{
		Out_ConStr = "FF Microsoft ODBC : Server instance shouldn't be empty !";
		return false;
	}

	Out_ConStr = "{SQL Server};SERVER=" + ODBC_Source + "\\" + ServerInstance + ";DSN=" + ODBC_Source + ";UID=" + Username + ";PWD=" + Password;
	return true;
}

bool AODBC_Manager::ConnectDatabase(FString& Out_Code, const FString& In_ConStr)
{
	SQLRETURN RetCode = SQLAllocEnv(&this->SQL_Handle_Environment);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : Failed to allocate SQL environment";
		return false;
	}

	RetCode = SQLSetEnvAttr(this->SQL_Handle_Environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : Failed to set the ODBC version.";
		SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);
		return false;
	}

	RetCode = SQLAllocConnect(this->SQL_Handle_Environment, &this->SQL_Handle_Connection);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : Failed to allocate a connection handle.";
		SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);
		return false;
	}

	RetCode = SQLDriverConnectA(this->SQL_Handle_Connection, NULL, (SQLCHAR*)TCHAR_TO_UTF8(*In_ConStr), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
	if (!SQL_SUCCEEDED(RetCode))
	{
		SQLFreeHandle(SQL_HANDLE_DBC, this->SQL_Handle_Connection);
		SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);

		Out_Code = "FF Microsoft ODBC : Connection couldn't made !";
		return false;
	}

	Out_Code = "FF Microsoft ODBC : Connection successfully established !";
	return true;
}

void AODBC_Manager::CreateConnection(FDelegate_ODBC_Connection DelegateConnection, const FString& In_ConStr)
{
	if (In_ConStr.IsEmpty())
	{
		DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Connection string shouldn't be empty !");
		return;
	}

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, In_ConStr]()
		{
			FString Out_Code;
			FString CreatedString;

			const bool ConnectionResult = this->ConnectDatabase(CreatedString, In_ConStr);
			this->ConnectionString = ConnectionResult ? In_ConStr : "";

			AsyncTask(ENamedThreads::GameThread, [DelegateConnection, ConnectionResult, CreatedString]()
				{
					DelegateConnection.ExecuteIfBound(ConnectionResult, CreatedString);
				}
			);
		}
	);
}

void AODBC_Manager::Disconnect()
{
	if (this->SQL_Handle_Connection)
	{
		SQLDisconnect(this->SQL_Handle_Connection);
		SQLFreeHandle(SQL_HANDLE_DBC, this->SQL_Handle_Connection);
		this->SQL_Handle_Connection = NULL;
	}

	if (this->SQL_Handle_Environment)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);
		this->SQL_Handle_Environment = NULL;
	}
}

int32 AODBC_Manager::ExecuteQuery(FODBC_QueryHandler& Out_Handler, FString& Out_Code, const FString& SQL_Query)
{
	if (SQL_Query.IsEmpty())
	{
		return 0;
	}

	if (!this->SQL_Handle_Connection)
	{
		Out_Code = "FF Microsoft ODBC : Connection handle is not valid !";
		return 0;
	}

	SQLHSTMT SQL_Handle;
	SQLRETURN RetCode = SQLAllocStmt(this->SQL_Handle_Connection, &SQL_Handle);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : There was a problem while allocating statement handle : " + FString::FromInt(RetCode);
		return 0;
	}

	SQLWCHAR* SQLWCHARStatementString = (SQLWCHAR*)(*SQL_Query);
	RetCode = SQLPrepare(SQL_Handle, SQLWCHARStatementString, SQL_NTS);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : There was a problem while preparing statement : " + FString::FromInt(RetCode);
		return 0;
	}

	RetCode = SQLSetStmtAttr(SQL_Handle, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_ON, 0);

	RetCode = SQLExecute(SQL_Handle);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : There was a problem while executing query : " + FString::FromInt(RetCode);
		return 0;
	}

	FODBC_QueryHandler Temp_Handler;
	Temp_Handler.SQL_Handle = SQL_Handle;
	Temp_Handler.SentQuery = SQL_Query;
	const int32 RecordResult = Temp_Handler.Record_Result(Out_Code);

	switch (RecordResult)
	{
		case 0: 
			return 0;
		
		case 1: 
			Out_Handler = Temp_Handler;
			return 1;
		
		case 2: 
			Out_Handler = Temp_Handler;
			return 2;

		default:
			return 0;
	}
}

void AODBC_Manager::ExecuteQueryBp(FDelegate_ODBC_Execute DelegateExecute, const FString& SQL_Query)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, DelegateExecute, SQL_Query]()
		{
			FString Out_Code;
			FODBC_QueryHandler Temp_Handler;
			const int32 ExecuteResult = this->ExecuteQuery(Temp_Handler, Out_Code, SQL_Query);

			AsyncTask(ENamedThreads::GameThread, [this, DelegateExecute, Out_Code, Temp_Handler, ExecuteResult]()
				{
					if (ExecuteResult == 1)
					{
						UODBC_Result* ResultObject = NewObject<UODBC_Result>();
						ResultObject->SetQueryResult(Temp_Handler);

						DelegateExecute.ExecuteIfBound(ExecuteResult, Out_Code, ResultObject, Temp_Handler.Affected_Rows);
					}

					else
					{
						DelegateExecute.ExecuteIfBound(ExecuteResult, Out_Code, nullptr, 0);
					}

				}
			);
		}
	);
}

FString AODBC_Manager::GetConnectionString()
{
	return this->ConnectionString;
}