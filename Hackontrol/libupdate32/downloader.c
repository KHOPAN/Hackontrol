#include <khopancurl.h>
#include "update.h"

static size_t write_data_stream(void* data, size_t size, size_t count, DataStream* stream) {
	size_t dataSize = size * count;

	if(!KHDataStreamAdd(stream, data, dataSize)) {
		return 0;
	}

	return dataSize;
}

BOOL DownloadLatestLibdll32(DataStream* stream) {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		return FALSE;
	}

	CURL* curl = curl_easy_init();
	BOOL returnValue = FALSE;

	if(!curl) {
		KHCURLDialogErrorW(CURLE_FAILED_INIT, L"curl_easy_init");
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, URL_LIBDLL32_FILE);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_stream);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
	}

	returnValue = TRUE;
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
	return returnValue;
}
