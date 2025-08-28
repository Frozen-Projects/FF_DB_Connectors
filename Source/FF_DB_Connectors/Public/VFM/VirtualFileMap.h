// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "VFM/VFM_Tools.h"

#include "VirtualFileMap.generated.h"

UCLASS()
class FF_DB_CONNECTORS_API UVirtualFileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:

	mutable FCriticalSection VFM_Guard;

#ifdef _WIN64

	TMap<FString, FVFM_Store> VirtualFileMaps;
	static FString GetErrorString(DWORD ErrorCode);

#endif // _WIN64

	virtual bool FileAddCallback(FString& Out_Code, FString FileName, const TArray<uint8>& FileData, TMap<FString, FString> Headers);
	virtual void FileRemoveCallback(FString FileName);
	virtual void CleanUpFileHandles();

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | Database Connectors | Virtual File Map")
	bool AddFile(FString& Out_Code, FString FileName, const TArray<uint8>& FileData, TMap<FString, FString> Headers, bool bAllowUpdate);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | Database Connectors | Virtual File Map")
	bool RemoveFile(FString& Out_Code, FString FileName);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | Database Connectors | Virtual File Map")
	bool FindOtherFiles(TArray<uint8>& Out_Buffer, FString& Out_Code, const FString& FileName);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest | Database Connectors | Virtual File Map")
	bool GetFiles(TMap<FString, FVFM_Export>& Out_Infos, FString& Out_Code);

};