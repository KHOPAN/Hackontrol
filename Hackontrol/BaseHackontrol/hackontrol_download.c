#include "hackontrolcurl.h"
#include <khopancurl.h>

#define SET_CODE if(outputCode)*outputCode=code

static size_t write_data_stream(void* const data, size_t size, size_t count, DataStream* const stream) {
	size_t dataSize = size * count;

	if(!KHDataStreamAdd(stream, data, dataSize)) {
		return 0;
	}

	return dataSize;
}

BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized, CURLcode* const outputCode) {
	CURLcode code = CURLE_OK;

	if(!stream || !url) {
		code = CURLE_BAD_FUNCTION_ARGUMENT;
		SET_CODE;
		return FALSE;
	}

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

BOOL HackontrolForceDownload(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized) {
	if(!stream | !url) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(!curlInitialized && curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		return FALSE;
	}

	CURL* curl = curl_easy_init();
	BOOL returnValue = FALSE;

	if(!curl) {
		SetLastError(ERROR_FUNCTION_FAILED);
		goto globalCleanup;
	}

	if(curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		goto easyCleanup;
	}

	if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_stream) != CURLE_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		goto easyCleanup;
	}

	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream) != CURLE_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		goto easyCleanup;
	}

	while(curl_easy_perform(curl) != CURLE_OK) {
		KHDataStreamFree(stream);
		Sleep(FORCE_DOWNLOAD_DELAY);
	}

	returnValue = TRUE;
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	if(!curlInitialized) {
		curl_global_cleanup();
	}

	return returnValue;
}
