// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "PSQL/PSQL_Result.h"

#include "PSQL_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_PSQL_Connection, bool, IsSuccessfull, FString, Out_Code);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_PSQL_Execute, int32, IsSuccessfull, FString, Out_Code);

UCLASS()
class FF_DB_CONNECTORS_API APSQL_Manager : public AActor
{
	GENERATED_BODY()

private:

	virtual bool ConnectDatabase(FString& Out_Code, const FString& ConnectionString);

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	
	// Sets default values for this actor's properties.
	APSQL_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|PSQL")
	virtual void CreateConnection(FDelegate_PSQL_Connection DelegateConnection, const FString& In_ConStr);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|PSQL")
	virtual void Disconnect();

	// Call this in FRunnableThread::Run() to execute a query. If there is a result, go to game thread and create a UODBC_Result object.
	//virtual int32 ExecuteQuery(FODBC_QueryHandler& Out_Handler, FString& Out_Code, const FString& SQL_Query);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|PSQL")
	virtual void ExecuteQueryBp(FDelegate_PSQL_Execute DelegateExecute, const FString& SQL_Query);

	UFUNCTION(BlueprintPure, Category = "Frozen Forest|Database Connectors|PSQL")
	virtual FString GetConnectionString();

};