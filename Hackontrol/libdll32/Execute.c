#include <Windows.h>
#include <khopancurl.h>
#include "execute.h"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		return;
	}
	
	cJSON* rootObject;

	if(!DownloadLatestJSON(&rootObject)) {
		goto globalCleanup;
	}

	char* json = cJSON_Print(rootObject);
	MessageBoxA(NULL, json, "libdll32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	free(json);
	cJSON_Delete(rootObject);
globalCleanup:
	curl_global_cleanup();
}
