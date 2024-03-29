#include <stdio.h>
#include "definition.h"

static size_t write_data(void* data, size_t size, size_t count, void* stream) {
	return fwrite(data, size, count, (FILE*) stream);
}

void HU_DownloadFile(CURL* curl, const char* url, const void* filePath, BOOL wide) {
	CURLcode code = curl_easy_setopt(curl, CURLOPT_URL, url);

	if(code != CURLE_OK) {
		HU_CURLError(code, "curl_easy_setopt()");
		ExitProcess(code);
		return;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		HU_CURLError(code, "curl_easy_setopt()");
		ExitProcess(code);
		return;
	}

	FILE* file = NULL;
	errno_t errorCode;

	if(wide) {
		errorCode = _wfopen_s(&file, filePath, L"wb");
	} else {
		errorCode = fopen_s(&file, filePath, "wb");
	}

	if(errorCode != 0 || file == NULL) {
		HU_DisplayError(errorCode, wide ? L"_wfopen_s()" : L"fopen_s");
		ExitProcess(errorCode);
		return;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		fclose(file);
		HU_CURLError(code, "curl_easy_setopt()");
		ExitProcess(code);
		return;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		fclose(file);
		HU_CURLError(code, "curl_easy_perform()");
		ExitProcess(code);
		return;
	}

	fclose(file);
}
