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
    this->InitializeCOM();
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

bool AOLEDB_Manager::Connect(FString ConnectionString)
{
	LPOLESTR WideString = const_cast<LPOLESTR>(*ConnectionString);
	
	IDataInitialize* pDataInit = nullptr;
	HRESULT Result = CoCreateInstance(CLSID_MSDAINITIALIZE, nullptr, CLSCTX_INPROC_SERVER, IID_IDataInitialize, (void**)&pDataInit);

	if (FAILED(Result) || !pDataInit)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create IDataInitialize instance. HRESULT: 0x%08X"), Result);
		return false;
	}

    IDBInitialize* pDBInit = nullptr;
    Result = pDataInit->GetDataSource(nullptr, CLSCTX_INPROC_SERVER, WideString, IID_IDBInitialize, (IUnknown**)&pDBInit);
    
    pDataInit->Release();
    
    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("GetDataSource failed: 0x%08X"), Result);
        return false;
    }

    Result = pDBInit->Initialize();
   
    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("IDBInitialize::Initialize failed: 0x%08X"), Result);
        pDBInit->Release();
        return false;
    }

    // Create session factory
    IDBCreateSession* pCreateSession = nullptr;
    Result = pDBInit->QueryInterface(IID_IDBCreateSession, (void**)&pCreateSession);
   
    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("QI(IDBCreateSession) failed: 0x%08X"), Result);
        pDBInit->Uninitialize();
        pDBInit->Release();
        return false;
    }

    // Create command factory
    IDBCreateCommand* pCreateCommand = nullptr;
    Result = pCreateSession->CreateSession(nullptr, IID_IDBCreateCommand, (IUnknown**)&pCreateCommand);
    
    if (FAILED(Result))
    {
        UE_LOG(LogTemp, Error, TEXT("IDBCreateSession::CreateSession failed: 0x%08X"), Result);
        pCreateSession->Release();
        pDBInit->Uninitialize();
        pDBInit->Release();
        return false;
    }

    // Store as opaque handles
    this->DB_Init = pDBInit;
    this->DB_Session = pCreateSession;
    this->DB_Command = pCreateCommand;

    UE_LOG(LogTemp, Display, TEXT("OLE DB connected."));
    return true;
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