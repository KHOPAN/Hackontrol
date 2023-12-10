#include <Windows.h>
#include "HackontrolUpdater.h"

CURL* HU_InitializeCURL() {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		HU_CURLError(code, "curl_global_init()");
		ExitProcess(code);
		return NULL;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		curl_global_cleanup();
		HU_CURLError(code, "curl_easy_init()");
		ExitProcess(-4);
		return NULL;
	}

	return curl;
}
