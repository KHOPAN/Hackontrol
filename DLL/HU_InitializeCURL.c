#include <Windows.h>
#include "definition.h"

CURL* HU_InitializeCURL() {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		curlError(code, L"curl_global_init");
		ExitProcess(code);
		return NULL;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		curl_global_cleanup();
		curlError(code, L"curl_easy_init");
		ExitProcess(-4);
		return NULL;
	}

	return curl;
}
