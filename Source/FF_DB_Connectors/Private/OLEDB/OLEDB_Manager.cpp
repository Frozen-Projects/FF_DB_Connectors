// Fill out your copyright notice in the Description page of Project Settings.

#include "OLEDB/OLEDB_Manager.h"

// Sets default values
AOLEDB_Manager::AOLEDB_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AOLEDB_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void AOLEDB_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AOLEDB_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOLEDB_Manager::ConnectTest()
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}