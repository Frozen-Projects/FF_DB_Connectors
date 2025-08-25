#include "ODBC/MS_ODBC_Manager.h"

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
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AODBC_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AODBC_Manager::CreateConnectionString(FString& Out_ConStr, FString TargetServer, FString Username, FString Password, FString ServerInstance)
{
	if (TargetServer.IsEmpty())
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

	Out_ConStr = "{SQL Server};SERVER=" + TargetServer + "\\" + ServerInstance + ";DSN=" + TargetServer + ";UID=" + Username + ";PWD=" + Password;
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

	this->ConnectionString = In_ConStr;

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection]()
		{
			FString Out_Code;
			FString CreatedString;

			if (!this->ConnectDatabase(Out_Code, this->ConnectionString))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, Out_Code]()
					{
						DelegateConnection.ExecuteIfBound(false, Out_Code);
					}
				);

				return;
			}
			
			AsyncTask(ENamedThreads::GameThread, [DelegateConnection, Out_Code]()
				{
					DelegateConnection.ExecuteIfBound(true, Out_Code);
				}
			);
		}
	);
}

FString AODBC_Manager::GetConnectionString()
{
	return this->ConnectionString;
}

bool AODBC_Manager::SendQuery(FString& Out_Code, UODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults)
{
	if (SQL_Query.IsEmpty())
	{
		return false;
	}

	if (!this->SQL_Handle_Connection)
	{
		Out_Code = "FF Microsoft ODBC : Connection handle is not valid !";
		return false;
	}

	SQLRETURN RetCode;

	SQLHSTMT Temp_Handle;
	RetCode = SQLAllocStmt(this->SQL_Handle_Connection, &Temp_Handle);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : There was a problem while allocating statement handle : " + FString::FromInt(RetCode);
		return false;
	}

	SQLWCHAR* SQLWCHARStatementString = (SQLWCHAR*)(*SQL_Query);
	RetCode = SQLPrepare(Temp_Handle, SQLWCHARStatementString, SQL_NTS);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : There was a problem while preparing statement : " + FString::FromInt(RetCode);
		return false;
	}

	RetCode = SQLExecute(Temp_Handle);

	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : There was a problem while executing query : " + FString::FromInt(RetCode);
		return false;
	}

	UODBC_Result* ResultObject = NewObject<UODBC_Result>();

	if (!ResultObject->SetQueryResult(Temp_Handle, SQL_Query))
	{
		Out_Code = "FF Microsoft ODBC : Query executed successfully but return handle is invalid !";
		return false;
	}

	if (bRecordResults)
	{
		FString RecordResultCode;
		if (!ResultObject->Result_Record(RecordResultCode))
		{
			Out_Code = "FF Microsoft ODBC : Query executed successfully but there was a problem while recording result to the pool : " + UKismetStringLibrary::ParseIntoArray(RecordResultCode, " : ")[1];
			return false;
		}
	}

	Out_Result = ResultObject;
	Out_Code = "FF Microsoft ODBC : Query executed and result object created successfully !";

	delete(SQLWCHARStatementString);

	return true;
}

void AODBC_Manager::SendQueryAsync(FDelegate_ODBC_Execute DelegateExecute, const FString& SQL_Query, bool bRecordResults)
{
	UODBC_Result* Out_Result = NewObject<UODBC_Result>();

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateExecute, &Out_Result, SQL_Query, bRecordResults]()
		{
			FString Out_Code;

			if (this->SendQuery(Out_Code, Out_Result, SQL_Query, bRecordResults))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code, Out_Result]()
					{
						DelegateExecute.ExecuteIfBound(true, Out_Code, Out_Result);
					}
				);

				return;
			}

			else
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code]()
					{
						DelegateExecute.ExecuteIfBound(false, Out_Code, nullptr);
					}
				);

				return;
			}
		}
	);
}