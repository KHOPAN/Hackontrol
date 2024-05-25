#include <hackontrolcurl.h>
#include "execute.h"

BOOL DownloadLatestJSON(cJSON** output) {
	DataStream stream = {0};
	CURLcode code;

	if(!HackontrolDownloadData(&stream, URL_LATEST_FILE, TRUE, &code)) {
		KHCURLDialogErrorW(code, L"HackontrolDownloadData");
		return FALSE;
	}

	KHDataStreamAdd(&stream, "", sizeof(CHAR));
	cJSON* root = cJSON_Parse(stream.data);
	KHDataStreamFree(&stream);

	if(!root) {
		MESSAGE_BOX(L"Error while parsing JSON document");
		return FALSE;
	}

	(*output) = root;
	return TRUE;
}
