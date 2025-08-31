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

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegate_ODBC_CI, bool, IsSuccessfull, FString, Out_Code, const TArray<FODBC_ColumnInfo>&, Out_Infos);

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

	// This will only executes query and return its handle without recording it. We use this generic function in both "ExecuteQuery" and "LearnColumns_Internal" functions to prevent boilerplate code.
	static SQLHSTMT ExecuteQuery_Internal(FString& Out_Code, SQLHDBC In_Connection, FCriticalSection* In_Guard, const FString& SQL_Query);

	// Internal side of "LearnColumnsBp" to make Async BP function cleaner.
	virtual bool LearnColumns_Internal(TArray<FODBC_ColumnInfo>& Out_ColumnInfos, FString& Out_Code, const FString& SQL_Query);

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

	virtual SQLHDBC GetConnectionHandle();
	virtual FCriticalSection* GetGuard();

	UFUNCTION(BlueprintPure, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual FString GetConnectionString();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual bool CreateConnectionString(FString& Out_ConStr, FString ODBC_Source, FString Username, FString Password, FString ServerInstance = "SQLEXPRESS");

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void CreateConnection(FDelegate_ODBC_Connection DelegateConnection, const FString& In_ConStr);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void Disconnect();

	/*
	* This will executes query and record its results.
	* You can call this in FRunnableThread::Run().
	* You should design like this this. If there is a result, go to game thread and create UODBC_Result object and set its result. (Check ExecuteQueryBp function as a sample.)
	*/
	static int32 ExecuteQuery(FODBC_QueryHandler& Out_Handler, FString& Out_Code, SQLHDBC In_Connection, FCriticalSection* In_Guard, const FString& SQL_Query);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void ExecuteQueryBp(FDelegate_ODBC_Execute DelegateExecute, const FString& SQL_Query);

	/*
	* This function will even work if table is empty and there is no result. It is important to know the column names and types to generate APIs.
	* SELECT * FROM dbo.YourTable WHERE 1=0; -- This query will return zero rows but will give column info.
	*/
	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void LearnColumnsBp(FDelegate_ODBC_CI DelegateColumnInfos, const FString& SQL_Query = "SELECT * FROM dbo.YourTable WHERE 1=0");

};