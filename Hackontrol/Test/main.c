#include <stdio.h>
#include <khopanwin32.h>
#include <hackontrol.h>

//#define ENVIRONMENT_VARIABLE L"ProgramFiles(x86)"

int main(int argc, char** argv) {
	/*DWORD size = GetEnvironmentVariableW(ENVIRONMENT_VARIABLE, NULL, 0);

	if(!size) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetEnvironmentVariableW");
		return 1;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalAlloc");
		return 1;
	}

	int returnValue = 1;

	if(!GetEnvironmentVariableW(ENVIRONMENT_VARIABLE, buffer, size)) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetEnvironmentVariableW");
		goto freeBuffer;
	}

	printf("%ws\n", buffer);
	returnValue = 0;
freeBuffer:
	if(LocalFree(buffer)) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalFree");
		returnValue = 1;
	}

	return returnValue;*/
	LPWSTR pathFolderHackontrol = GetHackontrolDirectory();

	if(!pathFolderHackontrol) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetHackontrolDirectory");
		return 1;
	}

	printf("Path: %ws\n", pathFolderHackontrol);

	if(LocalFree(pathFolderHackontrol)) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalFree");
		return 1;
	}

	return 0;
}
