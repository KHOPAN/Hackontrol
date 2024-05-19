#include "execute.h"

static size_t write_data(BYTE* data, size_t size, size_t count, void** output);

BOOL DownloadLatestJSON(cJSON** output) {
	CURL* curl = curl_easy_init();

	if(!curl) {
		KHCURLDialogErrorW(CURLE_FAILED_INIT, L"curl_easy_init");
		return FALSE;
	}

	CURLcode code = curl_easy_setopt(curl, CURLOPT_URL, URL_LATEST_FILE);
	BOOL returnValue = FALSE;

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	BYTE* latestFile = NULL;
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &latestFile);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
		goto easyCleanup;
	}

	cJSON* root = cJSON_Parse(latestFile);
	FREE(latestFile);

	if(!root) {
		MESSAGE_BOX(L"Error while parsing JSON document");
		goto easyCleanup;
	}

	(*output) = root;
	returnValue = TRUE;
easyCleanup:
	curl_easy_cleanup(curl);
	return returnValue;
}

static size_t write_data(BYTE* data, size_t size, size_t count, void** output) {
	if(!output) {
		return 0;
	}

	BYTE* outputBuffer = *output;
	size_t length = size * count;

	if(!outputBuffer) {
		BYTE* result = LocalAlloc(LMEM_FIXED, length + 1);

		if(!result) {
			KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
			return 0;
		}

		for(size_t i = 0; i < length; i++) {
			result[i] = data[i];
		}

		result[length] = 0;
		*output = result;
		return length;
	}

	size_t previousLength = strlen(outputBuffer);
	size_t resultLength = previousLength + length;
	BYTE* result = LocalAlloc(LMEM_FIXED, resultLength + 1);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		return 0;
	}

	for(size_t i = 0; i < previousLength; i++) {
		result[i] = outputBuffer[i];
	}

	FREE(outputBuffer);

	for(size_t i = 0; i < length; i++) {
		result[i + previousLength] = data[i];
	}

	result[resultLength] = 0;
	*output = result;
	return length;
}
