#include "HackontrolUpdater.h"

void HU_DownloadFile(CURL* curl, const char* url, const wchar_t* filePath) {
	CURLcode code = curl_easy_setopt(curl, CURLOPT_URL, url);

	if(code != CURLE_OK) {
		HU_CURLError(code, "curl_easy_setopt()");
		ExitProcess(code);
		return;
	}

	FILE* file = NULL;
	errno_t errorCode = _wfopen_s(&file, filePath, L"wb");
	
	if(errorCode != NULL || file == NULL) {
		HU_DisplayError(errorCode, L"_wfopen_s()");
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
