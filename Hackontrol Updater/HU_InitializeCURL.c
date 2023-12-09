#include <Windows.h>
#include "HackontrolUpdater.h"

CURL* HU_InitializeCURL() {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		ExitProcess(code);
		return NULL;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		ExitProcess(-1);
		return NULL;
	}

	return curl;
}
