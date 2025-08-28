// Fill out your copyright notice in the Description page of Project Settings.

#include "OLEDB/OLEDB_Manager.h"
#include "OLEDB/OLEDB_Includes.h"

// Sets default values.
AOLEDB_Manager::AOLEDB_Manager()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AOLEDB_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void AOLEDB_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	this->Disconnect();
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AOLEDB_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AOLEDB_Manager::ConnectDatabase(FString& OutCode, const FString& In_ConStr)
{
    if (In_ConStr.IsEmpty())
    {
		OutCode = "FF Microsoft OLE DB : Connection string shouldn't be empty !";
        return false;
    }

    if (!this->bCOMInitialized)
    {
        HRESULT Result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        this->bCOMInitialized = SUCCEEDED(Result) || Result == RPC_E_CHANGED_MODE;

        if (FAILED(Result))
        {
            OutCode = "FF Microsoft OLE DB : Failed to initialize COM library.";
            return false;
        }
    }

	LPOLESTR WideString = const_cast<LPOLESTR>(*In_ConStr);
	
	IDataInitialize* pDataInit = nullptr;
	HRESULT Result = CoCreateInstance(CLSID_MSDAINITIALIZE, nullptr, CLSCTX_INPROC_SERVER, IID_IDataInitialize, (void**)&pDataInit);

	if (FAILED(Result) || !pDataInit)
	{
		OutCode = FString::Printf(TEXT("FF Microsoft OLE DB : Failed to create IDataInitialize instance. HRESULT: 0x%08X"), Result);
		return false;
	}

    IDBInitialize* pDBInit = nullptr;
    Result = pDataInit->GetDataSource(nullptr, CLSCTX_INPROC_SERVER, WideString, IID_IDBInitialize, (IUnknown**)&pDBInit);
    
    pDataInit->Release();
    
    if (FAILED(Result))
    {
		OutCode = FString::Printf(TEXT("FF Microsoft OLE DB : GetDataSource failed. HRESULT: 0x%08X"), Result);
        return false;
    }

    Result = pDBInit->Initialize();
   
    if (FAILED(Result))
    {
        pDBInit->Release();

		OutCode = FString::Printf(TEXT("FF Microsoft OLE DB : IDBInitialize::Initialize failed. HRESULT: 0x%08X"), Result);
        return false;
    }

    // Create session factory
    IDBCreateSession* pCreateSession = nullptr;
    Result = pDBInit->QueryInterface(IID_IDBCreateSession, (void**)&pCreateSession);
   
    if (FAILED(Result))
    {
        pDBInit->Uninitialize();
        pDBInit->Release();

		OutCode = FString::Printf(TEXT("FF Microsoft OLE DB : QI(IDBCreateSession) failed. HRESULT: 0x%08X"), Result);
        return false;
    }

    // Create command factory
    IDBCreateCommand* pCreateCommand = nullptr;
    Result = pCreateSession->CreateSession(nullptr, IID_IDBCreateCommand, (IUnknown**)&pCreateCommand);
    
    if (FAILED(Result))
    {
        pCreateSession->Release();
        pDBInit->Uninitialize();
        pDBInit->Release();

		OutCode = FString::Printf(TEXT("FF Microsoft OLE DB : IDBCreateSession::CreateSession failed. HRESULT: 0x%08X"), Result);
        return false;
    }

    // Store as opaque handles
    this->DB_Init = pDBInit;
    this->DB_Session = pCreateSession;
    this->DB_Command = pCreateCommand;

	OutCode = FString("FF Microsoft OLE DB : Connection successful.");
    return true;
}

FString AOLEDB_Manager::CreateConnectionString(FString ServerIp, FString Database, FString UserID, FString Password, FString Provider, int32 Port, bool bEnableEncryption, bool bTrustCertificate)
{
    if (ServerIp.IsEmpty() || Database.IsEmpty() || UserID.IsEmpty())
    {
        return FString();
    }

    const FString TempString =
        "Provider=" + (Provider.IsEmpty() ? "MSOLEDBSQL" : Provider) + ";" +
        "Data Source=" + ServerIp + "," + (Port == 0 ? FString::FromInt(1433) : FString::FromInt(Port)) + ";" +
        "Initial Catalog=" + Database + ";" +
        "User ID=" + UserID + ";" +
        "Password=" + Password + ";" +
        "Encrypt=" + (bEnableEncryption ? "Yes" : "No") + ";" +
        "TrustServerCertificate=" + (bTrustCertificate ? "Yes" : "No") + ";";

    return TempString;
}

void AOLEDB_Manager::CreateConnection(FDelegate_OLEDB_Connection DelegateConnection, const FString& In_ConStr)
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

            if (!this->ConnectDatabase(Out_Code, In_ConStr))
            {
                this->ConnectionString = In_ConStr;

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

void AOLEDB_Manager::Disconnect()
{
    if (this->DB_Command)
    {
        ((IDBCreateCommand*)this->DB_Command)->Release();
        this->DB_Command = nullptr;
    }

    if (this->DB_Session)
    {
        ((IDBCreateSession*)this->DB_Session)->Release();
        this->DB_Session = nullptr;
    }

    if (this->DB_Init)
    {
        IDBInitialize* pDBInit = (IDBInitialize*)this->DB_Init;
        pDBInit->Uninitialize();
        pDBInit->Release();
        this->DB_Init = nullptr;
    }

    if (this->bCOMInitialized)
    {
        CoUninitialize();
        this->bCOMInitialized = false;
    }
}

int32 AOLEDB_Manager::ExecuteQuery(void*& RowSetBuffer, int64& AffectedRows, FString& Out_Code, const FString& Query)
{
    if (Query.IsEmpty())
    {
		Out_Code = "FF Microsoft OLE DB : Query string shouldn't be empty !";
		RowSetBuffer = nullptr;
        return 0;
    }

    if (!this->DB_Command)
    {
		Out_Code = "FF Microsoft OLE DB : Database is not connected !";
		RowSetBuffer = nullptr;
        return 0;
    }

    ICommandText* pCommandText = nullptr;
    HRESULT Result = ((IDBCreateCommand*)this->DB_Command)->CreateCommand(nullptr, IID_ICommandText, (IUnknown**)&pCommandText);

    if (FAILED(Result))
    {
		Out_Code = FString::Printf(TEXT("FF Microsoft OLE DB : CreateCommand failed. HRESULT: 0x%08X"), Result);
		RowSetBuffer = nullptr;
        return 0;
    }

    Result = pCommandText->SetCommandText(DBGUID_DBSQL, const_cast<LPOLESTR>(*Query));

    if (FAILED(Result))
    {
        pCommandText->Release();

		Out_Code = FString::Printf(TEXT("FF Microsoft OLE DB : SetCommandText failed. HRESULT: 0x%08X"), Result);
		RowSetBuffer = nullptr;
        return 0;
    }

    IRowset* pRowset = nullptr;
    DBROWCOUNT cRowsAffected = 0;
    Result = pCommandText->Execute(nullptr, IID_IRowset, nullptr, &cRowsAffected, (IUnknown**)&pRowset);
	AffectedRows = cRowsAffected < 0 ? 0 : (int64)cRowsAffected;

    pCommandText->Release();

    if (FAILED(Result))
    {
        Out_Code = FString::Printf(TEXT("FF Microsoft OLE DB : Execute failed. HRESULT: 0x%08X"), Result);
        RowSetBuffer = nullptr;
        return 0;
    }

    else if (SUCCEEDED(Result) && !pRowset)
    {
        Out_Code = "Query executed successfully, but no columns were returned. It is update only.";
        RowSetBuffer = nullptr;
        return 2;
    }

    else
    {
        RowSetBuffer = reinterpret_cast<void*>(pRowset);
        Out_Code = "Query executed successfully.";
        return 1;
    }
}

void AOLEDB_Manager::ExecuteQueryBp(FDelegate_OLEDB_Execute DelegateExecute, const FString& Query)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateExecute, Query]()
        {
            void* RowSetBuffer = nullptr;
            int64 Affected_Rows = 0;
            FString Out_Code;

            int32 Result = this->ExecuteQuery(RowSetBuffer, Affected_Rows, Out_Code, Query);

            switch (Result)
            {
                case 0:

                    AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code]()
                        {
                            DelegateExecute.ExecuteIfBound(0, Out_Code, nullptr, 0);
                        }
				    );

                    return;

                case 1:

                    AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code, RowSetBuffer, Affected_Rows]()
                        {
						    UOLEDB_Result* ResultObject = NewObject<UOLEDB_Result>();
                            ResultObject->SetRowSetBuffer(RowSetBuffer);
						    DelegateExecute.ExecuteIfBound(1, Out_Code, ResultObject, Affected_Rows);
                        }
                    );

                    return;

			    case 2:

                    AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code, Affected_Rows]()
                        {
                            DelegateExecute.ExecuteIfBound(2, Out_Code, nullptr, Affected_Rows);
                        }
                    );

                    return;

                default:

                    AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code]()
                        {
                            DelegateExecute.ExecuteIfBound(0, Out_Code, nullptr, 0);
                        }
                    );

                    return;
            }
        }
	);
}

FString AOLEDB_Manager::GetConnectionString()
{
    return this->ConnectionString;
}