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

	if(!CheckAndProcessSelfUpdate(rootObject)) {
		goto deleteJson;
	}

	ProcessFilesArray(rootObject);
	ProcessEntrypointsArray(rootObject);
deleteJson:
	cJSON_Delete(rootObject);
globalCleanup:
	curl_global_cleanup();
}
