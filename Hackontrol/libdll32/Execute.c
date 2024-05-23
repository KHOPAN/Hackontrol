#include "execute.h"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		return;
	}
	
	cJSON* rootObject;

	/*if(!DownloadLatestJSON(&rootObject)) {
		goto globalCleanup;
	}/**/

	HANDLE file = CreateFileW(L"D:\\GitHub Repository\\Hackontrol\\system\\latest.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		goto globalCleanup;
	}

	LARGE_INTEGER fileSize;
	BOOL match = FALSE;

	if(!GetFileSizeEx(file, &fileSize)) {
		CloseHandle(file);
		goto globalCleanup;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, fileSize.QuadPart);

	if(!buffer) {
		CloseHandle(file);
		goto globalCleanup;
	}

	DWORD bytesRead;

	if(!ReadFile(file, buffer, fileSize.LowPart, &bytesRead, NULL)) {
		LocalFree(buffer);
		CloseHandle(file);
		goto globalCleanup;
	}

	CloseHandle(file);

	if(fileSize.LowPart != bytesRead) {
		LocalFree(buffer);
		goto globalCleanup;
	}

	rootObject = cJSON_Parse(buffer);
	LocalFree(buffer);/**/

	/*if(!CheckAndProcessSelfUpdate(rootObject)) {
		goto deleteJson;
	}/**/

	ProcessFilesArray(rootObject);
	ProcessEntrypointsArray(rootObject);
deleteJson:
	cJSON_Delete(rootObject);
globalCleanup:
	curl_global_cleanup();
}
