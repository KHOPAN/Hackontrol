#include "hackontrol.h"
#include <khopancurl.h>

#define SET_CODE if(outputCode)*outputCode=code

static size_t write_data_stream(void* const data, size_t size, size_t count, DataStream* const stream);

BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR url, BOOL curlInitialized, CURLcode* const outputCode) {
	if(!stream || !url) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	CURLcode code = CURLE_OK;
	BOOL returnValue = FALSE;

	if(!curlInitialized) {
		code = curl_global_init(CURL_GLOBAL_ALL);

		if(code != CURLE_OK) {
			SET_CODE;
			return FALSE;
		}
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, url);

	if(code != CURLE_OK) {
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_stream);

	if(code != CURLE_OK) {
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if(code != CURLE_OK) {
		goto easyCleanup;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		goto easyCleanup;
	}

	returnValue = TRUE;
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	if(!curlInitialized) {
		curl_global_cleanup();
	}

	SET_CODE;
	return returnValue;
}

static size_t write_data_stream(void* const data, size_t size, size_t count, DataStream* const stream) {
	size_t dataSize = size * count;

	if(!KHDataStreamAdd(stream, data, dataSize)) {
		return 0;
	}

	return dataSize;
}
