// Fill out your copyright notice in the Description page of Project Settings.

#include "LMDB/LMDB_Manager.h"

// Sets default values
ALMDB_Manager::ALMDB_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void ALMDB_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void ALMDB_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void ALMDB_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ALMDB_Manager::MDB_OpenDB(FString In_Path, int32 DatabaseNumber)
{
	if (this->bIsDbOpened)
	{
		return false;
	}

	if (In_Path.IsEmpty())
	{
		return false;
	}

	FPaths::NormalizeFilename(In_Path);

	const FString DB_Name = FPaths::GetBaseFilename(In_Path, true);
	FString DB_Path = FPaths::GetPath(In_Path);

	if (!FPaths::DirectoryExists(DB_Path))
	{
		return false;
	}

	FPaths::MakePlatformFilename(DB_Path);

	MDB_txn* Transaction = nullptr;
	MDB_val Key, Value;

	memset(&Key, 0, sizeof(MDB_val));
	memset(&Value, 0, sizeof(MDB_val));

	int Result = mdb_env_create(&this->LMDB_Environment);

	if (DatabaseNumber > 1)
	{
		Result = mdb_env_set_maxdbs(this->LMDB_Environment, DatabaseNumber);
	}

	// 0664 is file permission flag for UNIX
	Result = mdb_env_open(this->LMDB_Environment, TCHAR_TO_UTF8(*DB_Path), 0, 0664);
	Result = mdb_txn_begin(this->LMDB_Environment, nullptr, 0, &Transaction);
	Result = mdb_dbi_open(Transaction, TCHAR_TO_UTF8(*DB_Name), 0, &this->Database);

	if (Result == MDB_SUCCESS)
	{
		this->bIsDbOpened = true;
		return true;
	}

	else
	{
		return false;
	}
}

bool ALMDB_Manager::MDB_WriteValue(TMap<FString, FString> In_Data)
{
	if (!this->bIsDbOpened)
	{
		return false;
	}

	if (In_Data.IsEmpty())
	{
		return false;
	}



	return false;
}
