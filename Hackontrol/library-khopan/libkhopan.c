#include "libkhopanhackontrol.h"
#include "libkhopanjava.h"
#include "libkhopanlist.h"
#include <ShlObj_core.h>

#define ERROR_WIN32(sourceName, functionName)             if(error){error->facility=ERROR_FACILITY_WIN32;error->code=GetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)
#define ERROR_SOURCE(sourceName)                          if(error){error->source=sourceName;}

#define HACKONTROL_DIRECTORY L"%LOCALAPPDATA%\\Microsoft\\InstallService"

#pragma warning(disable: 6386)

typedef void(__stdcall* RUNDLL32FUNCTION) (HWND window, HINSTANCE instance, LPSTR argument, int command);

typedef struct {
	RUNDLL32FUNCTION function;
	LPSTR argument;
	HANDLE thread;
} RUNDLL32DATA, *PRUNDLL32DATA;

void InternalKHOPANError(const UINT facility, const ULONG code, const LPCWSTR source, const KHOPANERRORDECODER decoder, const BOOL console) {
	KHOPANERROR error = {0};
	error.facility = facility;
	error.code = code;
	error.source = source;
	InternalKHOPANErrorKHOPAN(&error, decoder, console);
}

void InternalKHOPANErrorKHOPAN(const PKHOPANERROR error, const KHOPANERRORDECODER decoder, const BOOL console) {
	LPWSTR message = KHOPANGetErrorMessage(error, decoder);

	if(message) {
		if(console) {
			_putws(message);
		} else {
			MessageBoxW(NULL, message, L"Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		}

		KHOPAN_DEALLOCATE(message);
	}
}

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
		message = NULL;
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

BOOL KHOPANCreateDirectory(const LPCWSTR folder) {
	if(!folder) {
		return FALSE;
	}

	return SHCreateDirectoryExW(NULL, folder, NULL) ? FALSE : TRUE;
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

	LPWSTR fileCmd = KHOPANFileGetCmd();

	if(!fileCmd) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANExecuteCommand", L"KHOPANFileGetCmd");
		return FALSE;
	}

	LPWSTR argument = KHOPANFormatMessage(L"%ws /c \"%ws\"", fileCmd, command);

	if(!argument) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANExecuteCommand", L"KHOPANFormatMessage");
		goto freeFileCmd;
	}

	STARTUPINFOW startup = {0};
	startup.cb = sizeof(STARTUPINFOW);
	PROCESS_INFORMATION process;

	if(!CreateProcessW(fileCmd, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startup, &process)) {
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
	KHOPAN_DEALLOCATE(fileCmd);
	ERROR_CLEAR;
	return TRUE;
freeArgument:
	KHOPAN_DEALLOCATE(argument);
freeFileCmd:
	KHOPAN_DEALLOCATE(fileCmd);
	return FALSE;
}

BOOL KHOPANExecuteDynamicLibrary(const LPCWSTR file, const LPCSTR function, const LPCSTR argument, const PKHOPANERROR error) {
	if(!file || !function) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANExecuteDynamicLibrary", NULL);
		return FALSE;
	}

	LPWSTR fileRundll32 = KHOPANFileGetRundll32();

	if(!fileRundll32) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANExecuteDynamicLibrary", L"KHOPANFileGetRundll32");
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
	}

	return response;
}

BOOL KHOPANExecuteProcess(const LPCWSTR file, const LPCWSTR argument, const BOOL block, const PKHOPANERROR error) {
	if(!file) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANExecuteProcess", NULL);
		return FALSE;
	}

	LPWSTR argumentMutable = KHOPANStringDuplicate(argument);
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

LPWSTR KHOPANFileGetCmd() {
	LPWSTR folderWindows = KHOPANFolderGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileCmd = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, folderWindows);
	KHOPAN_DEALLOCATE(folderWindows);
	return fileCmd;
}

LPWSTR KHOPANFileGetRundll32() {
	LPWSTR folderWindows = KHOPANFolderGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileRundll32 = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, folderWindows);
	KHOPAN_DEALLOCATE(folderWindows);
	return fileRundll32;
}

LPWSTR KHOPANFolderGetWindows() {
	UINT size = GetSystemWindowsDirectoryW(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = KHOPAN_ALLOCATE(size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		KHOPAN_DEALLOCATE(buffer);
		return NULL;
	}

	return buffer;
}

LPWSTR KHOPANStringDuplicate(const LPCWSTR text) {
	if(!text) {
		return NULL;
	}

	size_t length = wcslen(text);

	if(length < 1) {
		return NULL;
	}

	LPWSTR buffer = KHOPAN_ALLOCATE((length + 1) * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	for(size_t i = 0; i < length; i++) {
		buffer[i] = text[i];
	}

	buffer[length] = 0;
	return buffer;
}

BOOL KHOPANWriteFile(const LPCWSTR file, const LPVOID data, const size_t size, const PKHOPANERROR error) {
	if(!file || !data || !size) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANWriteFile", NULL);
		return FALSE;
	}

	HANDLE handle = CreateFileW(file, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(handle == INVALID_HANDLE_VALUE) {
		ERROR_WIN32(L"KHOPANWriteFile", L"CreateFileW");
		return FALSE;
	}

	size_t pointer = 0;
	DWORD written = 0;

	while(pointer < size) {
		size_t remaining = size - pointer;

		if(!WriteFile(handle, ((PBYTE) data) + pointer, (DWORD) min(remaining, MAXDWORD), &written, NULL)) {
			ERROR_WIN32(L"KHOPANWriteFile", L"WriteFile");
			CloseHandle(handle);
			return FALSE;
		}

		pointer += written;
	}

	CloseHandle(handle);
	ERROR_CLEAR;
	return TRUE;
}

LPWSTR KHOPANHackontrolGetHomeDirectory() {
	DWORD size = ExpandEnvironmentStringsW(HACKONTROL_DIRECTORY, NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = KHOPAN_ALLOCATE(size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!ExpandEnvironmentStringsW(HACKONTROL_DIRECTORY, buffer, size)) {
		KHOPAN_DEALLOCATE(buffer);
		return NULL;
	}

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

BOOL KHOPANStreamInitialize(const PDATASTREAM stream, const size_t size, const PKHOPANERROR error) {
	if(!stream) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamInitialize", NULL);
		return FALSE;
	}

	if(size) {
		stream->data = KHOPAN_ALLOCATE(size);

		if(!stream->data) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANStreamInitialize", L"KHOPAN_ALLOCATE");
			return FALSE;
		}
	} else {
		stream->data = NULL;
	}

	stream->size = 0;
	stream->capacity = size;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANStreamAdd(const PDATASTREAM stream, const LPVOID data, const size_t size, const PKHOPANERROR error) {
	if(!stream || !data || !size) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamAdd", NULL);
		return FALSE;
	}

	size_t length = stream->size + size;
	size_t index;

	if(length > stream->capacity) {
		LPVOID buffer = KHOPAN_ALLOCATE(length);

		if(!buffer) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANStreamAdd", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		if(stream->data) {
			for(index = 0; index < stream->size; index++) ((PBYTE) buffer)[index] = ((PBYTE) stream->data)[index];
			KHOPAN_DEALLOCATE(stream->data);
		}

		stream->capacity = length;
		stream->data = buffer;
	}

	for(index = 0; index < size; index++) {
		((PBYTE) stream->data)[stream->size + index] = ((PBYTE) data)[index];
	}

	stream->size += size;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANStreamFree(const PDATASTREAM stream, const PKHOPANERROR error) {
	if(!stream) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamFree", NULL);
		return FALSE;
	}

	stream->size = 0;
	stream->capacity = 0;

	if(stream->data) {
		KHOPAN_DEALLOCATE(stream->data);
		stream->data = NULL;
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayInitialize(const PARRAYLIST list, const size_t size, const PKHOPANERROR error) {
	if(!list || !size) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayInitialize", NULL);
		return FALSE;
	}

	LPVOID buffer = KHOPAN_ALLOCATE(size * KHOPAN_ARRAY_INITIAL_CAPACITY);

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANArrayInitialize", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	list->count = 0;
	list->size = size;
	list->capacity = KHOPAN_ARRAY_INITIAL_CAPACITY;
	list->data = buffer;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayAdd(const PARRAYLIST list, const LPVOID data, const PKHOPANERROR error) {
	if(!list || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayAdd", NULL);
		return FALSE;
	}

	LPVOID buffer;
	size_t index;

	if(list->count >= list->capacity) {
		size_t size = list->size * list->capacity;
		buffer = KHOPAN_ALLOCATE(size * KHOPAN_ARRAY_SCALE_FACTOR);

		if(!buffer) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANArrayAdd", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		for(index = 0; index < size; index++) {
			((PBYTE) buffer)[index] = ((PBYTE) list->data)[index];
		}

		KHOPAN_DEALLOCATE(list->data);
		list->capacity *= KHOPAN_ARRAY_SCALE_FACTOR;
		list->data = buffer;
	}

	buffer = ((PBYTE) list->data) + list->size * list->count;

	for(index = 0; index < list->size; index++) {
		((PBYTE) buffer)[index] = ((PBYTE) data)[index];
	}

	list->count++;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index, const PKHOPANERROR error) {
	if(!list) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayRemove", NULL);
		return FALSE;
	}

	if(index >= list->count) {
		ERROR_COMMON(ERROR_COMMON_INDEX_OUT_OF_BOUNDS, L"KHOPANArrayRemove", NULL);
		return FALSE;
	}

	list->count--;

	if(!list->count) {
		ERROR_CLEAR;
		return TRUE;
	}

	LPVOID buffer = ((PBYTE) list->data) + list->size * index;

	for(size_t i = 0; i < (list->count - index) * list->size; i++) {
		((PBYTE) buffer)[i] = ((PBYTE) buffer)[i + list->size];
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, LPVOID* const data, const PKHOPANERROR error) {
	if(!list || !data || !list->count) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayGet", NULL);
		return FALSE;
	}

	if(index >= list->count) {
		ERROR_COMMON(ERROR_COMMON_INDEX_OUT_OF_BOUNDS, L"KHOPANArrayGet", NULL);
		return FALSE;
	}

	*data = ((PBYTE) list->data) + list->size * index;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayFree(const PARRAYLIST list, const PKHOPANERROR error) {
	if(!list) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayFree", NULL);
		return FALSE;
	}

	list->count = 0;
	list->size = 0;
	list->capacity = 0;

	if(list->data) {
		KHOPAN_DEALLOCATE(list->data);
		list->data = NULL;
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANLinkedInitialize(const PLINKEDLIST list, const size_t size, const PKHOPANERROR error) {
	if(!list || !size) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANLinkedInitialize", NULL);
		return FALSE;
	}

	list->count = 0;
	list->size = size;
	list->first = NULL;
	list->last = NULL;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANLinkedAdd(const PLINKEDLIST list, const LPVOID data, const PPLINKEDLISTITEM item, const PKHOPANERROR error) {
	if(!list || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANLinkedAdd", NULL);
		return FALSE;
	}

	PLINKEDLISTITEM buffer = KHOPAN_ALLOCATE(sizeof(LINKEDLISTITEM));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANLinkedAdd", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	buffer->data = KHOPAN_ALLOCATE(list->size);

	if(!buffer->data) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPANLinkedAdd", L"KHOPAN_ALLOCATE");
		KHOPAN_DEALLOCATE(buffer);
		return FALSE;
	}

	for(size_t i = 0; i < list->size; i++) {
		((PBYTE) buffer->data)[i] = ((PBYTE) data)[i];
	}

	buffer->list = list;
	buffer->previous = NULL;
	buffer->next = NULL;

	if(list->last) {
		list->last->next = buffer;
		buffer->previous = list->last;
	} else {
		list->first = buffer;
	}

	list->last = buffer;

	if(item) {
		*item = buffer;
	}

	list->count++;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANLinkedRemove(const PLINKEDLISTITEM item, const PKHOPANERROR error) {
	if(!item) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANLinkedRemove", NULL);
		return FALSE;
	}

	PLINKEDLIST list = item->list;

	if(!list) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANLinkedRemove", NULL);
		return FALSE;
	}

	if(list->first == item) {
		list->first = item->next;
	}

	if(list->last == item) {
		list->last = item->previous;
	}

	if(item->previous) {
		item->previous->next = item->next;
	}

	if(item->next) {
		item->next->previous = item->previous;
	}

	if(item->data) {
		KHOPAN_DEALLOCATE(item->data);
	}

	KHOPAN_DEALLOCATE(item);
	list->count--;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANLinkedGet(const PLINKEDLIST list, const size_t index, const PPLINKEDLISTITEM item, const PKHOPANERROR error) {
	if(!list || !item) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANLinkedGet", NULL);
		return FALSE;
	}

	if(index >= list->count) {
		ERROR_COMMON(ERROR_COMMON_INDEX_OUT_OF_BOUNDS, L"KHOPANLinkedGet", NULL);
		return FALSE;
	}

	size_t count = 0;
	BOOL found = FALSE;
	PLINKEDLISTITEM listItem;

	KHOPAN_LINKED_LIST_ITERATE_FORWARD(listItem, list) {
		if(index == count) {
			*item = listItem;
			found = TRUE;
			break;
		}

		count++;
	}

	if(!found) {
		ERROR_COMMON(ERROR_COMMON_INDEX_OUT_OF_BOUNDS, L"KHOPANLinkedGet", NULL);
		return FALSE;
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANLinkedFree(const PLINKEDLIST list, const PKHOPANERROR error) {
	if(!list) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANLinkedFree", NULL);
		return FALSE;
	}

	PLINKEDLISTITEM item = list->last;

	while(item) {
		PLINKEDLISTITEM buffer = item->previous;

		if(item->data) {
			KHOPAN_DEALLOCATE(item->data);
		}

		KHOPAN_DEALLOCATE(item);
		item = buffer;
	}

	list->count = 0;
	list->size = 0;
	list->first = NULL;
	list->last = NULL;
	ERROR_CLEAR;
	return TRUE;
}
