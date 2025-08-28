#include "PSQL/PSQL_Manager.h"

// Sets default values.
APSQL_Manager::APSQL_Manager()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void APSQL_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void APSQL_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	this->Disconnect();
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void APSQL_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool APSQL_Manager::ConnectDatabase(FString& OutCode, const FString& In_ConStr)
{
    return true;
}

void APSQL_Manager::CreateConnection(FDelegate_PSQL_Connection DelegateConnection, const FString& In_ConStr)
{

}

void APSQL_Manager::Disconnect()
{

}

/*
int32 APSQL_Manager::ExecuteQuery(void*& RowSetBuffer, int64& AffectedRows, FString& Out_Code, const FString& Query)
{
    return 0;
}
*/

void APSQL_Manager::ExecuteQueryBp(FDelegate_PSQL_Execute DelegateExecute, const FString& Query)
{

}

FString APSQL_Manager::GetConnectionString()
{
	return FString();
}