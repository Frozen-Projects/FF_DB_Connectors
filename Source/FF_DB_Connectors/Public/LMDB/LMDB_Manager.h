// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LMDB/LMDB_Includes.h"

#include "LMDB_Manager.generated.h"

UCLASS()
class FF_DB_CONNECTORS_API ALMDB_Manager : public AActor
{
	GENERATED_BODY()

private:

	MDB_env* LMDB_Environment = nullptr;
	MDB_dbi Database;
	bool bIsDbOpened = false;

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	ALMDB_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual bool MDB_OpenDB(FString In_Path, int32 DatabaseNumber);

	UFUNCTION(BlueprintCallable)
	virtual bool MDB_WriteValue(TMap<FString, FString> In_Data);

};
