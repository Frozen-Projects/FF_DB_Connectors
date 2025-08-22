// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "OLEDB/OLEDB_Result.h"

#include "OLEDB_Manager.generated.h"

UCLASS()
class FF_DB_CONNECTORS_API AOLEDB_Manager : public AActor
{
	GENERATED_BODY()
	
private:

	void* DB_Init = nullptr;
	void* DB_Session = nullptr;
	void* DB_Command = nullptr;
	bool bCOMInitialized = false;

	virtual bool InitializeCOM();
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

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual bool Connect(FString ConnectionString);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual bool ExecuteOnly(FString Query);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual bool ExecuteAndGetResult(UOLEDB_Result*& OutResult, FString Query);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|Database Connectors|OLEDB")
	virtual void Disconnect();

};