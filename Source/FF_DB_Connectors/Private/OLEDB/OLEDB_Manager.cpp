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

bool AOLEDB_Manager::InitializeCOM()
{
	HRESULT Result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	this->bCOMInitialized = SUCCEEDED(Result) || Result == RPC_E_CHANGED_MODE;
	return this->bCOMInitialized;
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
        if (!this->InitializeCOM())
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

bool AOLEDB_Manager::SendQuery(void*& RowSetBuffer, FString Query)
{
    if (Query.IsEmpty())
    {
        return false;
    }

    if (!this->DB_Command)
    {
        return false;
    }

    ICommandText* pCommandText = nullptr;
    HRESULT Result = ((IDBCreateCommand*)this->DB_Command)->CreateCommand(nullptr, IID_ICommandText, (IUnknown**)&pCommandText);

    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("CreateCommand failed: 0x%08X"), Result);
        return false;
    }

    Result = pCommandText->SetCommandText(DBGUID_DBSQL, const_cast<LPOLESTR>(*Query));

    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("SetCommandText failed: 0x%08X"), Result);
        pCommandText->Release();
        return false;
    }

    IRowset* pRowset = nullptr;

    Result = pCommandText->Execute(nullptr, IID_IRowset, nullptr, nullptr, (IUnknown**)&pRowset);
    pCommandText->Release();

    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("Execute failed: 0x%08X"), Result);
        return false;
    }

	RowSetBuffer = reinterpret_cast<void*>(pRowset);
    return true;
}

bool AOLEDB_Manager::ExecuteOnly(FString Query)
{
	void* pRowset = nullptr;

    if (!this->SendQuery(pRowset, Query))
    {
        return false;
    }

    if (!pRowset)
    {
        return false;
    }

    return true;
}

bool AOLEDB_Manager::ExecuteAndGetResult(UOLEDB_Result*& OutResult, FString Query)
{
    void* pRowset = nullptr;

    if (!this->SendQuery(pRowset, Query))
    {
        return false;
    }

    if (!pRowset)
    {
        return false;
    }

    UOLEDB_Result* ResultObject = NewObject<UOLEDB_Result>();
    bool bSetResult = ResultObject->SetRowSetBuffer(reinterpret_cast<void*>(pRowset));

    if (bSetResult)
    {
        OutResult = ResultObject;
		return true;
    }

    else
    {
        return false;
    }
}