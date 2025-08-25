// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "ODBC/MS_ODBC_Result.h"

#include "MS_ODBC_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_ODBC_Connection, bool, IsSuccessfull, FString, Out_Code);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegate_ODBC_Execute, bool, IsSuccessfull, FString, Out_Code, UODBC_Result*, Out_Result);

UCLASS()
class FF_DB_CONNECTORS_API AODBC_Manager : public AActor
{
	GENERATED_BODY()
	
protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FString ConnectionString;
	SQLHENV SQL_Handle_Environment = NULL;
	SQLHDBC SQL_Handle_Connection = NULL;

	virtual bool ConnectDatabase(FString& Out_Code, const FString& ConnectionString);

public:	
	
	// Sets default values for this actor's properties.
	AODBC_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual bool CreateConnectionString(FString& ConnectionString, FString TargetServer, FString Username, FString Password, FString ServerInstance = "SQLEXPRESS");

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void CreateConnection(FDelegate_ODBC_Connection DelegateConnection, const FString& ConnectionString);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual bool SendQuery(FString& Out_Code, UODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual void SendQueryAsync(FDelegate_ODBC_Execute DelegateExecute, const FString& SQL_Query, bool bRecordResults);

	UFUNCTION(BlueprintPure, Category = "Frozen Forest|Database Connectors|ODBC")
	virtual FString GetConnectionString();

};