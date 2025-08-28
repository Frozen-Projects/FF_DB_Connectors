#include "VFM/VirtualFileMap.h"

void UVirtualFileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UVirtualFileSubsystem::Deinitialize()
{
	this->CleanUpFileHandles();
	Super::Deinitialize();
}

#ifdef _WIN64
FString UVirtualFileSubsystem::GetErrorString(DWORD ErrorCode)
{
	char* msgBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ErrorCode, 0, (LPSTR)&msgBuffer, 0, nullptr);

	std::string message(msgBuffer, size);
	LocalFree(msgBuffer);
	
	return (FString)UTF8_TO_TCHAR(message.c_str());
}
#endif // _WIN64

bool UVirtualFileSubsystem::FileAddCallback(FString& Out_Code, FString FileName, const TArray<uint8>& FileData, TMap<FString, FString> Headers)
{
#ifdef _WIN64

	FScopeLock Lock(&this->VFM_Guard);

	const wchar_t* FileNameChar = *FileName;
	const size_t BufferSize = FileData.Num();

	HANDLE TempHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(BufferSize), FileNameChar);
	DWORD LastError = GetLastError();

	if (!TempHandle)
	{
		Out_Code = "Failed to create file mapping at: " + FileName + " : " + UVirtualFileSubsystem::GetErrorString(LastError);
		return false;
	}

	if (LastError == ERROR_ALREADY_EXISTS)
	{
		Out_Code = "File mapping already exists at: " + FileName + " : " + UVirtualFileSubsystem::GetErrorString(LastError);

		CloseHandle(TempHandle);
		return false;
	}

	// Map the file to memory
	void* TempBuffer = MapViewOfFile(TempHandle, FILE_MAP_ALL_ACCESS, 0, 0, BufferSize);

	if (!TempBuffer)
	{
		LastError = GetLastError();
		Out_Code = "Failed to map view of file at: " + FileName + " : " + UVirtualFileSubsystem::GetErrorString(LastError);

		CloseHandle(TempHandle);
		UnmapViewOfFile(TempBuffer);
		return false;
	}

	// Copy the data to the mapped memory. If we move, we can lose original content.
	FMemory::Memcpy(TempBuffer, FileData.GetData(), BufferSize);

	// We need to store the handle and the pointer to the mapped memory so we can unmap and close it later and prevent dangling handles and pointers.
	FVFM_Store FileStruct;
	FileStruct.MappedMemory = TempBuffer;
	FileStruct.MappedSize = BufferSize;
	FileStruct.MappingHandle = TempHandle;

	if (!Headers.IsEmpty())
	{
		FJsonObjectWrapper TempWrapper;
		
		for (const TPair<FString, FString>& Each_Header : Headers)
		{
			FJsonObjectWrapper Each_Header_Json;
			Each_Header_Json.JsonObject->SetStringField(Each_Header.Key, Each_Header.Value);
			TempWrapper.JsonObject->SetObjectField(Each_Header.Key, Each_Header_Json.JsonObject);
		}

		FileStruct.Headers = TempWrapper;
	}

	this->VirtualFileMaps.Add(FileName, FileStruct);
	FScopeLock Unlock(&this->VFM_Guard);

	Out_Code = TEXT("File added successfully.");
	return true;

#else
	return false;
#endif // _WIN64
}

void UVirtualFileSubsystem::FileRemoveCallback(FString FileName)
{
#ifdef _WIN64
	
	FScopeLock Lock(&this->VFM_Guard);

	if (FVFM_Store* FileHandle = this->VirtualFileMaps.Find(FileName))
	{
		if (FileHandle->MappedMemory)
		{
			UnmapViewOfFile(FileHandle->MappedMemory);
			FileHandle->MappedMemory = nullptr;
		}

		if (FileHandle->MappingHandle)
		{
			CloseHandle(FileHandle->MappingHandle);
			FileHandle->MappingHandle = nullptr;
		}

		FileHandle->MappedSize = 0;
		FileHandle->Headers = FJsonObjectWrapper();

		this->VirtualFileMaps.Remove(FileName);
	}

	FScopeLock Unlock(&this->VFM_Guard);

#else
	return;
#endif // _WIN64
}

void UVirtualFileSubsystem::CleanUpFileHandles()
{
#ifdef _WIN64
	
	FScopeLock Lock(&this->VFM_Guard);

	for (TPair<FString, FVFM_Store>& Pair : this->VirtualFileMaps)
	{
		FVFM_Store& FileHandle = Pair.Value;

		if (FileHandle.MappedMemory)
		{
			UnmapViewOfFile(FileHandle.MappedMemory);
			FileHandle.MappedMemory = nullptr;
		}

		if (FileHandle.MappingHandle)
		{
			CloseHandle(FileHandle.MappingHandle);
			FileHandle.MappingHandle = nullptr;
		}

		FileHandle.MappedSize = 0;
		FileHandle.Headers = FJsonObjectWrapper();
	}

	this->VirtualFileMaps.Empty();
	FScopeLock Unlock(&this->VFM_Guard);

#else
	return;
#endif // _WIN64
}

bool UVirtualFileSubsystem::AddFile(FString&Out_Code, FString FileName, const TArray<uint8>& FileData, TMap<FString, FString> Headers, bool bAllowUpdate)
{
#ifdef _WIN64

	if (FileName.IsEmpty())
	{
		Out_Code = TEXT("FileName is empty");
		return false;
	}

	if (FileData.IsEmpty() || !FileData.GetData())
	{
		Out_Code = TEXT("FileData is empty or invalid");
		return false;
	}

	if (!this->VirtualFileMaps.Contains(FileName))
	{
		return this->FileAddCallback(Out_Code, FileName, FileData, Headers);
	}

	else if (bAllowUpdate)
	{
		this->FileRemoveCallback(FileName);
		return this->FileAddCallback(Out_Code, FileName, FileData, Headers);
	}

	Out_Code = "File already exists and update is not allowed.";
	return false;

#else
	return false;
#endif // _WIN64
}

bool UVirtualFileSubsystem::RemoveFile(FString& Out_Code, FString FileName)
{
#ifdef _WIN64
	
	if (FileName.IsEmpty())
	{
		Out_Code = TEXT("FileName is empty");
		return false;
	}

	if (this->VirtualFileMaps.IsEmpty())
	{
		Out_Code = TEXT("No files in the virtual file map.");
		return false;
	}

	if (!this->VirtualFileMaps.Contains(FileName))
	{
		Out_Code = TEXT("File does not exist in the virtual file map.");
		return false;
	}

	this->FileRemoveCallback(FileName);

	Out_Code = TEXT("File removed successfully.");
	return true;

#else
	return false;
#endif // _WIN64
}

bool UVirtualFileSubsystem::FindOtherFiles(TArray<uint8>& Out_Buffer, FString& Out_Code, const FString& FileName)
{
#ifdef _WIN64

	if (FileName.IsEmpty())
	{
		Out_Code = TEXT("FileName is empty.");
		return false;
	}

	const wchar_t* NameW = *FileName;

	// 1) Open the named mapping created by another process
	HANDLE Mapping = OpenFileMappingW(FILE_MAP_READ, false, NameW);

	if (!Mapping)
	{
		const DWORD Error = GetLastError();
		Out_Code = "OpenFileMappingW failed : " + UVirtualFileSubsystem::GetErrorString(Error);
		return false;
	}

	// 2) Map the whole section for reading (dwNumberOfBytesToMap = 0 => entire section)
	void* View = MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, 0);

	if (!View)
	{
		const DWORD Error = GetLastError();
		Out_Code = "MapViewOfFile failed : " + UVirtualFileSubsystem::GetErrorString(Error);

		CloseHandle(Mapping);
		return false;
	}

	// 3) Determine the size of the mapped view. We accumulate RegionSize across contiguous regions that share the same AllocationBase.
	BYTE* Cursor = static_cast<BYTE*>(View);
	MEMORY_BASIC_INFORMATION Mbi{};
	SIZE_T TotalSize = 0;

	// AllocationBase of the first region corresponds to the base of this view
	if (VirtualQuery(View, &Mbi, sizeof(Mbi)) == 0)
	{
		UnmapViewOfFile(View);
		CloseHandle(Mapping);

		const DWORD Error = GetLastError();
		Out_Code = "VirtualQuery failed: " + UVirtualFileSubsystem::GetErrorString(Error);

		return false;
	}

	void* const AllocationBase = Mbi.AllocationBase;

	// Walk forward until AllocationBase changes (end of this view)
	while (VirtualQuery(Cursor, &Mbi, sizeof(Mbi)) != 0 && Mbi.AllocationBase == AllocationBase)
	{
		TotalSize += Mbi.RegionSize;
		Cursor += Mbi.RegionSize;
	}

	if (TotalSize == 0)
	{
		Out_Code = TEXT("Mapped view size is 0.");
		UnmapViewOfFile(View);
		CloseHandle(Mapping);
		return false;
	}

	// 4) Copy bytes out
	Out_Buffer.Empty();
	Out_Buffer.SetNumUninitialized(static_cast<int32>(TotalSize));
	FMemory::Memcpy(Out_Buffer.GetData(), View, TotalSize);

	// 5) Cleanup
	UnmapViewOfFile(View);
	CloseHandle(Mapping);

	Out_Code = TEXT("External mapped file read successfully.");
	return true;

#else
	return false;
#endif
}

bool UVirtualFileSubsystem::GetFiles(TMap<FString, FVFM_Export>& Out_Infos, FString& Out_Code)
{
	if (this->VirtualFileMaps.IsEmpty())
	{
		Out_Code = TEXT("No files in the virtual file map.");
		return false;
	}

	Out_Infos.Empty();

	for (const TPair<FString, FVFM_Store> Each_File : this->VirtualFileMaps)
	{
		const FString Each_File_Name = Each_File.Key;
		
		FVFM_Export Each_File_Info;
		Each_File_Info.MappedSize = Each_File.Value.MappedSize;
		Each_File_Info.Headers = Each_File.Value.Headers;

		Each_File_Info.Buffer.SetNumUninitialized(Each_File.Value.MappedSize);
		FMemory::Memcpy(Each_File_Info.Buffer.GetData(), Each_File.Value.MappedMemory, Each_File.Value.MappedSize);

		Out_Infos.Add(Each_File_Name, Each_File_Info);
	}

	return true;
}