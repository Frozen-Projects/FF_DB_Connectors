// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "OLEDB/OLEDB_Result.h"

#include "OLEDB_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_OLEDB_Connection, bool, IsSuccessfull, FString, Out_Code);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_FourParams(FDelegate_OLEDB_Execute, bool, IsSuccessfull, FString, Out_Code, UOLEDB_Result*, Out_Result, int64, Out_Affected);

UCLASS()
class FF_DB_CONNECTORS_API AOLEDB_Manager : public AActor
{
	GENERATED_BODY()
	
private:

	void* DB_Init = nullptr;
	void* DB_Session = nullptr;
	void* DB_Command = nullptr;
	bool bCOMInitialized = false;
	FString ConnectionString;

	virtual bool InitializeCOM();
	virtual bool ConnectDatabase(FString& OutCode, const FString& ConnectionString);
	virtual bool SendQuery(void*& RowSetBuffer, FString Query);

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	AOLEDB_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	/*
	* @param Port : Default SQL Server port is 1433. If you set it to 0, it will be 1433.
	*/
	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual FString CreateConnectionString(FString ServerIp, FString Database, FString UserID, FString Password, FString Provider = "MSOLEDBSQL", int32 Port = 1433, bool bEnableEncryption = true, bool bTrustCertificate = true);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual void CreateConnection(FDelegate_OLEDB_Connection DelegateConnection, const FString& In_ConStr);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual bool ExecuteOnly(FString Query);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual bool ExecuteAndGetResult(UOLEDB_Result*& OutResult, FString Query);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual void Disconnect();

};