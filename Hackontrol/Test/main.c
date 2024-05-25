#include <stdio.h>
#include <khopanwin32.h>
#include <hackontrol.h>

//#define ENVIRONMENT_VARIABLE L"ProgramFiles(x86)"

int main(int argc, char** argv) {
	/*const LPWSTR inputText = L"%SystemRoot%\\System32\\rundll32.dll";
	DWORD size = ExpandEnvironmentStringsW(inputText, NULL, 0);

	if(!size) {
		KHWin32ConsoleErrorW(GetLastError(), L"ExpandEnvironmentStringsW");
		return 1;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalAlloc");
		return 1;
	}

	int returnValue = 1;

	if(!ExpandEnvironmentStringsW(inputText, buffer, size)) {
		KHWin32ConsoleErrorW(GetLastError(), L"ExpandEnvironmentStringsW");
		goto freeBuffer;
	}

	printf("%ws\n", buffer);
	returnValue = 0;
freeBuffer:
	if(LocalFree(buffer)) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalFree");
		returnValue = 1;
	}

	return returnValue;*/
	DataStream stream = {0};
	CURLcode code;

	if(!HackontrolDownloadData(&stream, "https://www.google.com/", FALSE, &code)) {
		KHCURLConsoleErrorW(code, L"HackontrolDownloadData");
		return 1;
	}

	if(!HackontrolWriteFile(L"C:\\Users\\puthi\\Downloads\\google.html", &stream)) {
		KHWin32ConsoleErrorW(GetLastError(), L"HackontrolWriteFile");
		goto freeStream;
	}

	/*HANDLE file = CreateFileW(L"C:\\Users\\puthi\\Downloads\\google.html", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32ConsoleErrorW(GetLastError(), L"CreateFileW");
		goto freeStream;
	}

	DWORD bytesWritten;

	if(!WriteFile(file, stream.data, (DWORD) stream.size, &bytesWritten, NULL)) {
		KHWin32ConsoleErrorW(GetLastError(), L"WriteFile");
		goto closeFile;
	}

	if(stream.size != bytesWritten) {
		KHWin32ConsoleErrorW(ERROR_FUNCTION_FAILED, L"WriteFile");
	}
closeFile:
	CloseHandle(file);*/
freeStream:
	KHDataStreamFree(&stream);
	return 0;
}
