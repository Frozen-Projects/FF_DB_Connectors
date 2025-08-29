// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "ODBC/ODBC_Result.h"

#include "ODBC_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_ODBC_Connection, bool, IsSuccessfull, FString, Out_Code);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_FourParams(FDelegate_ODBC_Execute, int32, IsSuccessfull, FString, Out_Code, UODBC_Result*, Out_Result, int64, Out_Affected);

UCLASS()
class FF_DB_CONNECTORS_API AODBC_Manager : public AActor
{
	GENERATED_BODY()

private:

	mutable FCriticalSection DB_Guard;

	FString ConnectionString;
	SQLHENV SQL_Handle_Environment = NULL;
	SQLHDBC SQL_Handle_Connection = NULL;

	virtual bool ConnectDatabase(FString& Out_Code, const FString& ConnectionString);

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	
	// Sets default values for this actor's properties.
	AODBC_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	// Call this in FRunnableThread::Run() to execute a query. If there is a result, go to game thread and create a UODBC_Result object.
	static int32 ExecuteQuery(FODBC_QueryHandler& Out_Handler, FString& Out_Code, SQLHDBC In_Connection, FCriticalSection* In_Guard, const FString& SQL_Query);

	virtual SQLHDBC GetConnectionHandle();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual bool CreateConnectionString(FString& Out_ConStr, FString ODBC_Source, FString Username, FString Password, FString ServerInstance = "SQLEXPRESS");

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void CreateConnection(FDelegate_ODBC_Connection DelegateConnection, const FString& In_ConStr);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void ExecuteQueryBp(FDelegate_ODBC_Execute DelegateExecute, const FString& SQL_Query);

	UFUNCTION(BlueprintPure, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual FString GetConnectionString();

};