#include "khopanwin32.h"

typedef void(__stdcall* Rundll32Function) (HWND window, HINSTANCE instance, LPSTR argument, int command);

typedef struct {
	Rundll32Function function;
	LPSTR argument;
} Rundll32Data;

static DWORD WINAPI rundll32Thread(_In_ LPVOID parameter) {
	if(!parameter) {
		return ERROR_INVALID_PARAMETER;
	}

	Rundll32Data* data = (Rundll32Data*) parameter;

	if(!data->function) {
		return ERROR_INVALID_PARAMETER;
	}

	data->function(NULL, GetModuleHandleW(NULL), data->argument, 0);
	LocalFree(data);
	return 0;
}

static BOOL executeRundll32Function(const void* moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded, BOOL wide) {
	if(!moduleName || !functionName) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HMODULE module;

	if(wide) {
		module = LoadLibraryW(moduleName);
	} else {
		module = LoadLibraryA(moduleName);
	}

	if(!module) {
		return FALSE;
	}

	Rundll32Function function = (Rundll32Function) GetProcAddress(module, functionName);

	if(!function) {
		return FALSE;
	}

	if(!threaded) {
		function(NULL, GetModuleHandleW(NULL), "", 0);
		return TRUE;
	}

	Rundll32Data* data = LocalAlloc(LMEM_FIXED, sizeof(Rundll32Data));

	if(!data) {
		return FALSE;
	}

	data->function = function;
	data->argument = argument;
	HANDLE thread = CreateThread(NULL, 0, rundll32Thread, data, 0, NULL);

	if(!thread) {
		return FALSE;
	}

	CloseHandle(thread);
	return TRUE;
}

BOOL KHWin32ExecuteRundll32FunctionA(const LPSTR moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded) {
	return executeRundll32Function(moduleName, functionName, argument, threaded, FALSE);
}

BOOL KHWin32ExecuteRundll32FunctionW(const LPWSTR moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded) {
	return executeRundll32Function(moduleName, functionName, argument, threaded, TRUE);
}
