#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolcurl.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

__declspec(dllexport) void __stdcall DownloadFile(HWND window, HINSTANCE instance, LPSTR argument, int command) {
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
		MessageBoxW(NULL, L"Invalid argument, expected:\n<url> <outputFile>", L"libdll32", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		FREE(arguments);
		return;
	}

	LPSTR downloadUrl = KHFormatMessageA("%ws", arguments[0]);

	if(!downloadUrl) {
		KHWin32DialogErrorW(GetLastError(), L"KHFormatMessageA");
		FREE(arguments);
		return;
	}

	DataStream stream = {0};
	CURLcode code;
	BOOL result = HackontrolDownloadData(&stream, downloadUrl, FALSE, &code);
	FREE(downloadUrl);

	if(!result) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		FREE(arguments);
		return;
	}

	result = HackontrolWriteFile(arguments[1], &stream);
	FREE(arguments);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolWriteFile");
	}
}
