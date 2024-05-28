#include "khopanwin32.h"

typedef void(__stdcall* Rundll32Function) (HWND window, HINSTANCE instance, LPSTR argument, int command);

static DWORD WINAPI rundll32Thread(_In_ LPVOID parameter) {
	if(!parameter) {
		return ERROR_INVALID_PARAMETER;
	}

	Rundll32Function function = (Rundll32Function) parameter;
	function(NULL, GetModuleHandleW(NULL), "", 0);
	return 0;
}

static BOOL executeRundll32Function(const void* moduleName, const LPSTR functionName, BOOL threaded, BOOL wide) {
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

	HANDLE thread = CreateThread(NULL, 0, rundll32Thread, function, 0, NULL);

	if(!thread) {
		return FALSE;
	}

	CloseHandle(thread);
	return TRUE;
}

BOOL KHWin32ExecuteRundll32FunctionA(const LPSTR moduleName, const LPSTR functionName, BOOL threaded) {
	return executeRundll32Function(moduleName, functionName, threaded, FALSE);
}

BOOL KHWin32ExecuteRundll32FunctionW(const LPWSTR moduleName, const LPSTR functionName, BOOL threaded) {
	return executeRundll32Function(moduleName, functionName, threaded, TRUE);
}
