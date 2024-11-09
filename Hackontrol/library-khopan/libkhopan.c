#include "libkhopan.h"

#define ERROR_COMMON(errorCode, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=errorCode;error->function=functionName;}
#define ERROR_WIN32(functionName)             if(error){error->facility=ERROR_FACILITY_HRESULT;error->code=HRESULT_FROM_WIN32(GetLastError());error->function=functionName;}
#define ERROR_CLEAR ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL)

#define SAFECALL(x) {DWORD internalError=GetLastError();x;SetLastError(internalError);}

typedef void(__stdcall* RUNDLL32FUNCTION) (HWND window, HINSTANCE instance, LPSTR argument, int command);

typedef struct {
	RUNDLL32FUNCTION function;
	LPSTR argument;
} RUNDLL32DATA, *PRUNDLL32DATA;

static DWORD WINAPI KHOPANExecuteRundll32FunctionThread(_In_ LPVOID parameter) {
	if(!parameter) {
		return 1;
	}

	PRUNDLL32DATA data = parameter;

	if(!data->function) {
		return 1;
	}

	data->function(NULL, GetModuleHandleW(NULL), data->argument, 0);
	LocalFree(data->argument);
	LocalFree(data);
	return 0;
}

BOOL KHOPANEnablePrivilege(const LPCWSTR privilege, const PKHOPANERROR error) {
	if(!privilege) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANEnablePrivilege");
		return FALSE;
	}

	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
		ERROR_WIN32(L"OpenProcessToken");
		return FALSE;
	}

	LUID identifier;

	if(!LookupPrivilegeValueW(NULL, privilege, &identifier)) {
		ERROR_WIN32(L"LookupPrivilegeValueW");
		CloseHandle(token);
		return FALSE;
	}

	TOKEN_PRIVILEGES privileges = {0};
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identifier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), NULL, NULL)) {
		ERROR_WIN32(L"AdjustTokenPrivileges");
		CloseHandle(token);
		return FALSE;
	}

	CloseHandle(token);
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANExecuteCommand(const LPCWSTR command, const BOOL block) {
	if(!command) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR fileCommandPrompt = KHOPANFileGetCmd();

	if(!fileCommandPrompt) {
		return FALSE;
	}

	LPWSTR argument = KHOPANFormatMessage(L"%ws /c \"%ws\"", fileCommandPrompt, command);

	if(!argument) {
		SAFECALL(LocalFree(fileCommandPrompt));
		return FALSE;
	}

	STARTUPINFOW startup = {0};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process;
	startup.cb = CreateProcessW(fileCommandPrompt, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startup, &process);
	SAFECALL(LocalFree(argument));
	SAFECALL(LocalFree(fileCommandPrompt));

	if(!startup.cb || (block && WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED) || !CloseHandle(process.hProcess) || !CloseHandle(process.hThread)) {
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANExecuteDynamicLibrary(const LPCWSTR file, const LPCSTR function, const LPCSTR argument) {
	if(!file || !function) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR fileRundll32 = KHOPANFileGetRundll32();

	if(!fileRundll32) {
		return FALSE;
	}

	LPWSTR argumentRundll32 = argument ? KHOPANFormatMessage(L"%ws \"%ws\" %S %S", fileRundll32, file, function, argument) : KHOPANFormatMessage(L"%ws \"%ws\" %S", fileRundll32, file, function);

	if(!argumentRundll32) {
		SAFECALL(LocalFree(fileRundll32));
		return FALSE;
	}

	BOOL response = KHOPANExecuteProcess(fileRundll32, argumentRundll32, FALSE);
	SAFECALL(LocalFree(argumentRundll32));
	SAFECALL(LocalFree(fileRundll32));
	return response;
}

BOOL KHOPANExecuteProcess(const LPCWSTR file, const LPCWSTR argument, const BOOL block) {
	if(!file) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR argumentMutable = KHOPANStringDuplicate(argument);
	STARTUPINFOW startup = {0};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process;
	startup.cb = CreateProcessW(file, argumentMutable, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup, &process);

	if(argumentMutable) {
		SAFECALL(LocalFree(argumentMutable));
	}

	if(!startup.cb || (block && WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED) || !CloseHandle(process.hProcess) || !CloseHandle(process.hThread)) {
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANExecuteRundll32Function(const LPWSTR file, const LPCSTR function, const LPCSTR argument, const BOOL block) {
	if(!file || !function) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HMODULE executable = LoadLibraryW(file);

	if(!executable) {
		return FALSE;
	}

	RUNDLL32FUNCTION functionRundll32 = (RUNDLL32FUNCTION) GetProcAddress(executable, function);

	if(!functionRundll32) {
		return FALSE;
	}

	size_t length = argument ? strlen(argument) : 0;
	LPSTR argumentDuplicate = LocalAlloc(LMEM_FIXED, length + 1);

	if(!argumentDuplicate) {
		return FALSE;
	}

	for(size_t i = 0; i < length; i++) {
		argumentDuplicate[i] = argument[i];
	}

	if(block) {
		functionRundll32(NULL, GetModuleHandleW(NULL), argumentDuplicate, 0);
		LocalFree(argumentDuplicate);
		SetLastError(ERROR_SUCCESS);
		return TRUE;
	}

	PRUNDLL32DATA data = LocalAlloc(LMEM_FIXED, sizeof(RUNDLL32DATA));

	if(!data) {
		SAFECALL(LocalFree(argumentDuplicate));
		return FALSE;
	}

	data->function = functionRundll32;
	data->argument = argumentDuplicate;
	HANDLE thread = CreateThread(NULL, 0, KHOPANExecuteRundll32FunctionThread, data, 0, NULL);

	if(!thread) {
		SAFECALL(LocalFree(data));
		SAFECALL(LocalFree(argumentDuplicate));
		return FALSE;
	}

	CloseHandle(thread);
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

LPWSTR KHOPANFileGetCmd() {
	LPWSTR folderWindows = KHOPANFolderGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileCommandPrompt = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, folderWindows);
	SAFECALL(LocalFree(folderWindows));
	return fileCommandPrompt;
}

LPWSTR KHOPANFileGetRundll32() {
	LPWSTR folderWindows = KHOPANFolderGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileRundll32 = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, folderWindows);
	SAFECALL(LocalFree(folderWindows));
	return fileRundll32;
}

LPWSTR KHOPANFolderGetWindows() {
	UINT size = GetSystemWindowsDirectoryW(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		SAFECALL(LocalFree(buffer));
		return NULL;
	}

	SetLastError(ERROR_SUCCESS);
	return buffer;
}

LPWSTR KHOPANFormatMessage(const LPCWSTR format, ...) {
	if(!format) {
		return NULL;
	}

	va_list list;
	va_start(list, format);
	int length = _vscwprintf(format, list);
	LPWSTR buffer = NULL;

	if(length == -1) {
		goto functionExit;
	}

	buffer = KHOPAN_ALLOCATE((((size_t) length) + 1) * sizeof(WCHAR));

	if(!buffer) {
		goto functionExit;
	}

	if(vswprintf_s(buffer, ((size_t) length) + 1, format, list) == -1) {
		KHOPAN_DEALLOCATE(buffer);
		buffer = NULL;
	}
functionExit:
	va_end(list);
	return buffer;
}

LPSTR KHOPANFormatANSI(const LPCSTR format, ...) {
	if(!format) {
		return NULL;
	}

	va_list list;
	va_start(list, format);
	int length = _vscprintf(format, list);
	LPSTR buffer = NULL;

	if(length == -1) {
		goto functionExit;
	}

	buffer = KHOPAN_ALLOCATE(((size_t) length) + 1);

	if(!buffer) {
		goto functionExit;
	}

	if(vsprintf_s(buffer, ((size_t) length) + 1, format, list) == -1) {
		KHOPAN_DEALLOCATE(buffer);
		buffer = NULL;
	}
functionExit:
	va_end(list);
	return buffer;
}

static LPWSTR commonFacility(const PKHOPANERROR error) {
	if(error->facility != ERROR_FACILITY_COMMON) {
		return L"Unrecognized error facility";
	}

	switch(error->code) {
	case ERROR_COMMON_SUCCESS:           return L"An operation completed successfully";
	case ERROR_COMMON_UNDEFINED:         return L"Undefined or unknown error";
	case ERROR_COMMON_FUNCTION_FAILED:   return L"Function has failed";
	case ERROR_COMMON_INVALID_PARAMETER: return L"Function parameter is invalid";
	default:                             return L"Undefined or unknown error";
	}
}

LPWSTR KHOPANGetErrorMessage(const PKHOPANERROR error) {
	if(!error) {
		return NULL;
	}

	LPWSTR message = NULL;

	if(error->facility == ERROR_FACILITY_HRESULT) {
		if(error->code & 0x20000000) {
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK | (error->code & 0x10000000 ? FORMAT_MESSAGE_FROM_HMODULE : 0), error->code & 0x10000000 ? LoadLibraryW(L"ntdll.dll") : NULL, error->code & (error->code & 0x10000000 ? ~0x10000000 : 0xFFFF), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR) &message, 0, NULL);
		}
	} else {
		message = commonFacility(error);
	}

	LPWSTR result;

	if(message) {
		result = error->function ? KHOPANFormatMessage(L"%ws() error occurred. Facility: 0x%04X Error code: 0x%08X Message:\n%ws", error->function, error->facility, error->code, message) : KHOPANFormatMessage(L"Facility: 0x%04X Error code: 0x%08X Message:\n%ws", error->facility, error->code, message);

		if(error->facility == ERROR_FACILITY_HRESULT) {
			LocalFree(message);
		}
	} else {
		result = error->function ? KHOPANFormatMessage(L"%ws() error occurred. Facility: 0x%04X Error code: 0x%08X", error->function, error->facility, error->code) : KHOPANFormatMessage(L"Facility: 0x%04X Error code: 0x%08X", error->facility, error->code);
	}

	return result;
}

LPWSTR KHOPANStringDuplicate(const LPCWSTR text) {
	if(!text) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	size_t length = wcslen(text);

	if(length < 1) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, (length + 1) * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	for(size_t i = 0; i < length; i++) {
		buffer[i] = text[i];
	}

	buffer[length] = 0;
	SetLastError(ERROR_SUCCESS);
	return buffer;
}
