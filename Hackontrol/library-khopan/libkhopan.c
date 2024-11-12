#include "libkhopancurl.h"
#include "libkhopanjava.h"

#define ERROR_WIN32(sourceName, functionName)             if(error){error->facility=ERROR_FACILITY_WIN32;error->code=GetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)
#define ERROR_SOURCE(sourceName)                          if(error){error->source=sourceName;}

typedef void(__stdcall* RUNDLL32FUNCTION) (HWND window, HINSTANCE instance, LPSTR argument, int command);

typedef struct {
	RUNDLL32FUNCTION function;
	LPSTR argument;
	HANDLE thread;
} RUNDLL32DATA, *PRUNDLL32DATA;

LPCWSTR KHOPANErrorCommonDecoder(const PKHOPANERROR error) {
	if(!error || error->facility != ERROR_FACILITY_COMMON) {
		return NULL;
	}

	switch(error->code) {
	case ERROR_COMMON_SUCCESS:             return L"An operation completed successfully";
	case ERROR_COMMON_FUNCTION_FAILED:     return L"The function has failed";
	case ERROR_COMMON_INVALID_PARAMETER:   return L"The function parameter is invalid";
	case ERROR_COMMON_ALLOCATION_FAILED:   return L"Failed to allocate memory";
	case ERROR_COMMON_INDEX_OUT_OF_BOUNDS: return L"The specified index is out of bounds";
	default:                               return L"Undefined or unknown error";
	}
}

LPWSTR KHOPANFormatMessage(const LPCWSTR format, ...) {
	if(!format) {
		return NULL;
	}

	va_list list;
	va_start(list, format);
	size_t length = (UINT) (_vscwprintf(format, list) + 1);
	LPWSTR buffer = NULL;

	if(!length) {
		goto functionExit;
	}

	buffer = KHOPAN_ALLOCATE(length * sizeof(WCHAR));

	if(!buffer) {
		goto functionExit;
	}

	if(vswprintf_s(buffer, length, format, list) == -1) {
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
	size_t length = (UINT) (_vscprintf(format, list) + 1);
	LPSTR buffer = NULL;

	if(!length) {
		goto functionExit;
	}

	buffer = KHOPAN_ALLOCATE(length);

	if(!buffer) {
		goto functionExit;
	}

	if(vsprintf_s(buffer, length, format, list) == -1) {
		KHOPAN_DEALLOCATE(buffer);
		buffer = NULL;
	}
functionExit:
	va_end(list);
	return buffer;
}

LPWSTR KHOPANGetErrorMessage(const PKHOPANERROR error, const KHOPANERRORDECODER decoder) {
	if(!error) {
		return NULL;
	}

	LPWSTR message = NULL;

	switch(error->facility) {
	case ERROR_FACILITY_WIN32:
	case ERROR_FACILITY_HRESULT:
	case ERROR_FACILITY_NTSTATUS:
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK | (error->facility == ERROR_FACILITY_NTSTATUS ? FORMAT_MESSAGE_FROM_HMODULE : 0), error->facility == ERROR_FACILITY_NTSTATUS ? LoadLibraryW(L"ntdll.dll") : NULL, error->facility == ERROR_FACILITY_HRESULT ? error->code & 0xFFFF : error->code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR) &message, 0, NULL);
		break;
	case ERROR_FACILITY_CURL:
		message = KHOPANFormatMessage(L"%S", curl_easy_strerror(error->code));
		break;
	default:
		if(decoder) {
			message = (LPWSTR) decoder(error);
		}

		break;
	}

	LPWSTR result;
	LPCWSTR source = error->source ? error->source : error->function;

	if(message) {
		if(source) {
			result = error->function ? KHOPANFormatMessage(L"%ws() error occurred. Function: %ws() Facility: 0x%04X Error code: 0x%08X Message:\n%ws", source, error->function, error->facility, error->code, message) : KHOPANFormatMessage(L"%ws() error occurred. Facility: 0x%04X Error code: 0x%08X Message:\n%ws", source, error->facility, error->code, message);
		} else {
			result = error->function ? KHOPANFormatMessage(L"Function: %ws() Facility: 0x%04X Error code: 0x%08X Message:\n%ws", error->function, error->facility, error->code, message) : KHOPANFormatMessage(L"Facility: 0x%04X Error code: 0x%08X Message:\n%ws", error->facility, error->code, message);
		}

		switch(error->facility) {
		case ERROR_FACILITY_WIN32:
		case ERROR_FACILITY_HRESULT:
		case ERROR_FACILITY_NTSTATUS:
			LocalFree(message);
			break;
		case ERROR_FACILITY_CURL:
			KHOPAN_DEALLOCATE(message);
			break;
		}
	} else {
		if(source) {
			result = error->function ? KHOPANFormatMessage(L"%ws() error occurred. Function: %ws() Facility: 0x%04X Error code: 0x%08X", source, error->function, error->facility, error->code) : KHOPANFormatMessage(L"%ws() error occurred. Facility: 0x%04X Error code: 0x%08X", source, error->facility, error->code);
		} else {
			result = error->function ? KHOPANFormatMessage(L"Function: %ws() Facility: 0x%04X Error code: 0x%08X", error->function, error->facility, error->code) : KHOPANFormatMessage(L"Facility: 0x%04X Error code: 0x%08X", error->facility, error->code);
		}
	}

	return result;
}

BOOL KHOPANEnablePrivilege(const LPCWSTR privilege, const PKHOPANERROR error) {
	if(!privilege) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANEnablePrivilege", NULL);
		return FALSE;
	}

	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
		ERROR_WIN32(L"KHOPANEnablePrivilege", L"OpenProcessToken");
		return FALSE;
	}

	LUID identifier;

	if(!LookupPrivilegeValueW(NULL, privilege, &identifier)) {
		ERROR_WIN32(L"KHOPANEnablePrivilege", L"LookupPrivilegeValueW");
		CloseHandle(token);
		return FALSE;
	}

	TOKEN_PRIVILEGES privileges = {0};
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identifier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), NULL, NULL)) {
		ERROR_WIN32(L"KHOPANEnablePrivilege", L"AdjustTokenPrivileges");
		CloseHandle(token);
		return FALSE;
	}

	CloseHandle(token);
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANExecuteCommand(const LPCWSTR command, const BOOL block, const PKHOPANERROR error) {
	if(!command) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANExecuteCommand", NULL);
		return FALSE;
	}

	LPWSTR fileCommandPrompt = KHOPANFileGetCmd(error);

	if(!fileCommandPrompt) {
		ERROR_SOURCE(L"KHOPANExecuteCommand");
		return FALSE;
	}

	LPWSTR argument = KHOPANFormatMessage(L"%ws /c \"%ws\"", fileCommandPrompt, command);

	if(!argument) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANExecuteCommand", L"KHOPANFormatMessage");
		goto freeFileCommandPrompt;
	}

	STARTUPINFOW startup = {0};
	startup.cb = sizeof(STARTUPINFOW);
	PROCESS_INFORMATION process;

	if(!CreateProcessW(fileCommandPrompt, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startup, &process)) {
		ERROR_WIN32(L"KHOPANExecuteCommand", L"CreateProcessW");
		goto freeArgument;
	}

	if(block && WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED) {
		ERROR_WIN32(L"KHOPANExecuteCommand", L"WaitForSingleObject");
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
		goto freeArgument;
	}

	CloseHandle(process.hProcess);
	CloseHandle(process.hThread);
	KHOPAN_DEALLOCATE(argument);
	KHOPAN_DEALLOCATE(fileCommandPrompt);
	ERROR_CLEAR;
	return TRUE;
freeArgument:
	KHOPAN_DEALLOCATE(argument);
freeFileCommandPrompt:
	KHOPAN_DEALLOCATE(fileCommandPrompt);
	return FALSE;
}

BOOL KHOPANExecuteDynamicLibrary(const LPCWSTR file, const LPCSTR function, const LPCSTR argument, const PKHOPANERROR error) {
	if(!file || !function) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANExecuteDynamicLibrary", NULL);
		return FALSE;
	}

	LPWSTR fileRundll32 = KHOPANFileGetRundll32(error);

	if(!fileRundll32) {
		ERROR_SOURCE(L"KHOPANExecuteDynamicLibrary");
		return FALSE;
	}

	LPWSTR argumentRundll32 = argument ? KHOPANFormatMessage(L"%ws \"%ws\" %S %S", fileRundll32, file, function, argument) : KHOPANFormatMessage(L"%ws \"%ws\" %S", fileRundll32, file, function);

	if(!argumentRundll32) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANExecuteDynamicLibrary", L"KHOPANFormatMessage");
		KHOPAN_DEALLOCATE(fileRundll32);
		return FALSE;
	}

	BOOL response = KHOPANExecuteProcess(fileRundll32, argumentRundll32, FALSE, error);
	KHOPAN_DEALLOCATE(argumentRundll32);
	KHOPAN_DEALLOCATE(fileRundll32);

	if(!response) {
		ERROR_SOURCE(L"KHOPANExecuteDynamicLibrary");
		return FALSE;
	}

	return TRUE;
}

BOOL KHOPANExecuteProcess(const LPCWSTR file, const LPCWSTR argument, const BOOL block, const PKHOPANERROR error) {
	if(!file) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANExecuteProcess", NULL);
		return FALSE;
	}

	LPWSTR argumentMutable = KHOPANStringDuplicate(argument, NULL);
	STARTUPINFOW startup = {0};
	startup.cb = sizeof(STARTUPINFOW);
	PROCESS_INFORMATION process;

	if(!CreateProcessW(file, argumentMutable, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup, &process)) {
		ERROR_WIN32(L"KHOPANExecuteProcess", L"CreateProcessW");

		if(argumentMutable) {
			KHOPAN_DEALLOCATE(argumentMutable);
		}

		return FALSE;
	}

	if(argumentMutable) {
		KHOPAN_DEALLOCATE(argumentMutable);
	}

	if(block && WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED) {
		ERROR_WIN32(L"KHOPANExecuteCommand", L"WaitForSingleObject");
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
		return FALSE;
	}

	CloseHandle(process.hProcess);
	CloseHandle(process.hThread);
	ERROR_CLEAR;
	return TRUE;
}

static DWORD WINAPI rundll32Thread(_In_ PRUNDLL32DATA data) {
	if(!data || !data->function) {
		return 1;
	}

	data->function(NULL, GetModuleHandleW(NULL), data->argument, 0);
	KHOPAN_DEALLOCATE(data->argument);
	CloseHandle(data->thread);
	KHOPAN_DEALLOCATE(data);
	return 0;
}

BOOL KHOPANExecuteRundll32Function(const LPWSTR file, const LPCSTR function, const LPCSTR argument, const BOOL block, const PKHOPANERROR error) {
	if(!file || !function) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANExecuteRundll32Function", NULL);
		return FALSE;
	}

	HMODULE executable = LoadLibraryW(file);

	if(!executable) {
		ERROR_WIN32(L"KHOPANExecuteRundll32Function", L"LoadLibraryW");
		return FALSE;
	}

	RUNDLL32FUNCTION functionRundll32 = (RUNDLL32FUNCTION) GetProcAddress(executable, function);

	if(!functionRundll32) {
		ERROR_WIN32(L"KHOPANExecuteRundll32Function", L"GetProcAddress");
		return FALSE;
	}

	size_t length = argument ? strlen(argument) : 0;
	LPSTR argumentDuplicate = KHOPAN_ALLOCATE(length + 1);

	if(!argumentDuplicate) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANExecuteRundll32Function", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	for(size_t i = 0; i < length; i++) {
		argumentDuplicate[i] = argument[i];
	}

	if(block) {
		functionRundll32(NULL, GetModuleHandleW(NULL), argumentDuplicate, 0);
		KHOPAN_DEALLOCATE(argumentDuplicate);
		ERROR_CLEAR;
		return TRUE;
	}

	PRUNDLL32DATA data = KHOPAN_ALLOCATE(sizeof(RUNDLL32DATA));

	if(!data) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANExecuteRundll32Function", L"KHOPAN_ALLOCATE");
		KHOPAN_DEALLOCATE(argumentDuplicate);
		return FALSE;
	}

	data->function = functionRundll32;
	data->argument = argumentDuplicate;
	data->thread = CreateThread(NULL, 0, rundll32Thread, data, 0, NULL);

	if(!data->thread) {
		ERROR_WIN32(L"KHOPANExecuteRundll32Function", L"CreateThread");
		KHOPAN_DEALLOCATE(data);
		KHOPAN_DEALLOCATE(argumentDuplicate);
		return FALSE;
	}

	ERROR_CLEAR;
	return TRUE;
}

LPWSTR KHOPANFileGetCmd(const PKHOPANERROR error) {
	LPWSTR folderWindows = KHOPANFolderGetWindows(error);

	if(!folderWindows) {
		ERROR_SOURCE(L"KHOPANFileGetCmd");
		return NULL;
	}

	LPWSTR fileCommandPrompt = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, folderWindows);
	KHOPAN_DEALLOCATE(folderWindows);

	if(!fileCommandPrompt) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANFileGetCmd", L"KHOPANFormatMessage");
		return NULL;
	}

	ERROR_CLEAR;
	return fileCommandPrompt;
}

LPWSTR KHOPANFileGetRundll32(const PKHOPANERROR error) {
	LPWSTR folderWindows = KHOPANFolderGetWindows(error);

	if(!folderWindows) {
		ERROR_SOURCE(L"KHOPANFileGetRundll32");
		return NULL;
	}

	LPWSTR fileRundll32 = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, folderWindows);
	KHOPAN_DEALLOCATE(folderWindows);

	if(!fileRundll32) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANFileGetRundll32", L"KHOPANFormatMessage");
		return NULL;
	}

	ERROR_CLEAR;
	return fileRundll32;
}

LPWSTR KHOPANFolderGetWindows(const PKHOPANERROR error) {
	UINT size = GetSystemWindowsDirectoryW(NULL, 0);

	if(!size) {
		ERROR_WIN32(L"KHOPANFolderGetWindows", L"GetSystemWindowsDirectoryW");
		return NULL;
	}

	LPWSTR buffer = KHOPAN_ALLOCATE(size * sizeof(WCHAR));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANFolderGetWindows", L"KHOPAN_ALLOCATE");
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		ERROR_WIN32(L"KHOPANFolderGetWindows", L"GetSystemWindowsDirectoryW");
		KHOPAN_DEALLOCATE(buffer);
		return NULL;
	}

	ERROR_CLEAR;
	return buffer;
}

LPWSTR KHOPANStringDuplicate(const LPCWSTR text, const PKHOPANERROR error) {
	if(!text) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStringDuplicate", NULL);
		return NULL;
	}

	size_t length = wcslen(text);

	if(length < 1) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStringDuplicate", NULL);
		return NULL;
	}

	LPWSTR buffer = KHOPAN_ALLOCATE((length + 1) * sizeof(WCHAR));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANStringDuplicate", L"KHOPAN_ALLOCATE");
		return NULL;
	}

	for(size_t i = 0; i < length; i++) {
		buffer[i] = text[i];
	}

	buffer[length] = 0;
	ERROR_CLEAR;
	return buffer;
}

void KHOPANJavaThrow(JNIEnv* environment, const LPCSTR class, const LPCWSTR message) {
	if(!environment || !class || !message) {
		return;
	}

	jclass classObject = (*environment)->FindClass(environment, class);

	if(!classObject) {
		return;
	}

	jmethodID constructor = (*environment)->GetMethodID(environment, classObject, "<init>", "(Ljava/lang/String;)V");

	if(!constructor) {
		return;
	}

	jobject object = (*environment)->NewObject(environment, classObject, constructor, (*environment)->NewString(environment, message, (jsize) wcslen(message)));

	if(object) {
		(*environment)->Throw(environment, (jthrowable) object);
	}
}
