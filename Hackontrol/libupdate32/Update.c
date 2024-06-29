#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolcurl.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FUNCTION_LIBDLL32 L"Execute"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR wideArgument = KHFormatMessageW(L"%S", argument);

	if(!wideArgument) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	int count;
	LPWSTR* arguments = CommandLineToArgvW(wideArgument, &count);
	FREE(wideArgument);

	if(!arguments) {
		KHWin32DialogErrorW(GetLastError(), L"CommandLineToArgvW");
		return;
	}

	if(count < 2) {
		MessageBoxW(NULL, L"Invalid argument, expected:\n<pid> <url>", L"libupdate32", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		LocalFree(arguments);
		return;
	}

	DWORD processIdentifier = (DWORD) _wtoll(arguments[0]);
	LPSTR downloadUrl = KHFormatMessageA("%ws", arguments[1]);
	LocalFree(arguments);

	if(!downloadUrl) {
		KHWin32DialogErrorW(GetLastError(), L"KHFormatMessageA");
		return;
	}

	DataStream stream = {0};
	CURLcode code;
	BOOL result = HackontrolForceDownload(&stream, downloadUrl, FALSE);
	LocalFree(downloadUrl);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolForceDownload");
		return;
	}

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD) atol(argument));

	if(process) {
		WaitForSingleObject(process, INFINITE);
		CloseHandle(process);
	}

	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolGetDirectory");
		goto freeStream;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FILE_LIBDLL32, pathFolderHackontrol);
	FREE(pathFolderHackontrol);

	if(!pathFileLibdll32) {
		KHWin32DialogErrorW(GetLastError(), L"KHFormatMessageW");
		goto freeStream;
	}

	if(!HackontrolWriteFile(pathFileLibdll32, &stream)) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolWriteFile");
		FREE(pathFileLibdll32);
		goto freeStream;
	}

	result = KHWin32StartDynamicLibraryW(pathFileLibdll32, FUNCTION_LIBDLL32, NULL);
	FREE(pathFileLibdll32);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32StartDynamicLibraryW");
	}
freeStream:
	KHDataStreamFree(&stream);
}
