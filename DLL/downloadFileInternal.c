#include <stdio.h>
#include "definition.h"

static size_t write_data(void* data, size_t size, size_t count, void* stream) {
	return fwrite(data, size, count, (FILE*) stream);
}

BOOL downloadFileInternal(CURL* curl, const char* url, const void* filePath, BOOL wide) {
	CURLcode code = curl_easy_setopt(curl, CURLOPT_URL, url);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		return FALSE;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		return FALSE;
	}

	FILE* file = NULL;
	errno_t errorCode;

	if(wide) {
		errorCode = _wfopen_s(&file, filePath, L"wb");
	} else {
		errorCode = fopen_s(&file, filePath, "wb");
	}

	if(errorCode != 0 || file == NULL) {
		dialogError(errorCode, wide ? L"_wfopen_s" : L"fopen_s");
		return FALSE;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		fclose(file);
		curlError(code, L"curl_easy_setopt");
		return FALSE;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		fclose(file);
		curlError(code, L"curl_easy_perform");
		return FALSE;
	}

	fclose(file);
	return TRUE;
}
