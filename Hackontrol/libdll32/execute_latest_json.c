#include "execute.h"
#include <khopandatastream.h>

static size_t write_data_stream(void* data, size_t size, size_t count, DataStream* stream) {
	size_t dataSize = size * count;

	if(!KHDataStreamAdd(stream, data, dataSize)) {
		return 0;
	}

	return dataSize;
}

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

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_stream);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	DataStream stream = {0};
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
		goto easyCleanup;
	}

	KHDataStreamAdd(&stream, "", sizeof(CHAR));
	cJSON* root = cJSON_Parse(stream.data);
	KHDataStreamFree(&stream);

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
