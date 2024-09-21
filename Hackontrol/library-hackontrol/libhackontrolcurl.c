#include "libhackontrolcurl.h"

static size_t addDataStream(void* const data, const size_t size, const size_t count, const PDATASTREAM stream) {
	size_t total = size * count;

	if(!KHOPANStreamAdd(stream, data, total)) {
		return 0;
	}

	return total;
}

CURLcode HackontrolDownload(const LPCSTR location, const PDATASTREAM stream, const BOOL initialized, const BOOL force) {
	if(!location || !stream) {
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}

	CURLcode code;

	if(!initialized) {
		code = curl_global_init(CURL_GLOBAL_ALL);

		if(code != CURLE_OK) {
			return code;
		}
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		code = CURLE_FAILED_INIT;
		goto cleanupGlobal;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, location);

	if(code != CURLE_OK) {
		goto cleanupEasy;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, addDataStream);

	if(code != CURLE_OK) {
		goto cleanupEasy;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if(code != CURLE_OK) {
		goto cleanupEasy;
	}

	if(!force) {
		code = curl_easy_perform(curl);

		if(code != CURLE_OK) {
			goto cleanupEasy;
		}

		code = CURLE_OK;
		goto cleanupEasy;
	}

	while((code = curl_easy_perform(curl)) != CURLE_OK) {
		KHOPANERRORMESSAGE_CURL(code, L"curl_easy_perform");
		KHOPANStreamFree(stream);
		Sleep(DELAY_FORCE_DOWNLOAD);
	}

	code = CURLE_OK;
cleanupEasy:
	curl_easy_cleanup(curl);
cleanupGlobal:
	if(!initialized) {
		curl_global_cleanup();
	}

	return code;
}
