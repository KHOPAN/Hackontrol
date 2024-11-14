#include "libkhopancurl.h"

#define ERROR_CURL(sourceName, functionName)              if(error){error->facility=ERROR_FACILITY_CURL;error->code=code;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

static size_t appendData(const LPVOID data, const size_t size, const size_t count, const PDATASTREAM stream) {
	size_t total = size * count;

	if(!KHOPANStreamAdd(stream, data, total, NULL)) {
		return 0;
	}

	return total;
}

BOOL KHOPANDownloadData(const PDATASTREAM stream, const LPCSTR address, const BOOL initialized, const BOOL force, const PKHOPANERROR error) {
	if(!stream || !address) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANDownloadData", NULL);
		return FALSE;
	}

	CURLcode code;

	if(!initialized) {
		code = curl_global_init(CURL_GLOBAL_ALL);

		if(code != CURLE_OK) {
			ERROR_CURL(L"KHOPANDownloadData", L"curl_global_init");
			return FALSE;
		}
	}

	CURL* curl = curl_easy_init();
	BOOL codeExit = FALSE;

	if(!curl) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"KHOPANDownloadData", L"curl_easy_init");
		goto cleanupGlobal;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if(code != CURLE_OK) {
		ERROR_CURL(L"KHOPANDownloadData", L"curl_easy_setopt");
		goto cleanupEasy;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, address);

	if(code != CURLE_OK) {
		ERROR_CURL(L"KHOPANDownloadData", L"curl_easy_setopt");
		goto cleanupEasy;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendData);

	if(code != CURLE_OK) {
		ERROR_CURL(L"KHOPANDownloadData", L"curl_easy_setopt");
		goto cleanupEasy;
	}

	code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

	if(code != CURLE_OK) {
		ERROR_CURL(L"KHOPANDownloadData", L"curl_easy_setopt");
		goto cleanupEasy;
	}

	if(!force) {
		code = curl_easy_perform(curl);

		if(code != CURLE_OK) {
			ERROR_CURL(L"KHOPANDownloadData", L"curl_easy_perform");
			goto cleanupEasy;
		}

		goto clearError;
	}

	while(curl_easy_perform(curl) != CURLE_OK) {
		KHOPANStreamFree(stream, NULL);
	}
clearError:
	ERROR_CLEAR;
	codeExit = TRUE;
cleanupEasy:
	curl_easy_cleanup(curl);
cleanupGlobal:
	if(!initialized) {
		curl_global_cleanup();
	}

	return codeExit;
}
