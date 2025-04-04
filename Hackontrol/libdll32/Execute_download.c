#include <libhackontrol.h>
#include <libhackontrolcurl.h>
#include "Execute.h"

static void download(const cJSON* const root, const LPCWSTR folderHackontrol) {
	if(!root || !cJSON_IsObject(root)) {
		return;
	}

	LPWSTR file = ExecuteGetFile(root, folderHackontrol);

	if(!file) {
		return;
	}

	if(ExecuteHashFileCheck(root, file)) {
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

	if(HackontrolDownload(url, &stream, TRUE, FALSE) != CURLE_OK) {
		goto freeFile;
	}

	HackontrolWriteFile(file, stream.data, stream.size);
	KHOPANStreamFree(&stream);
freeFile:
	LocalFree(file);
}

void ExecuteDownload(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* fileField = cJSON_GetObjectItem(root, "file");

	if(!fileField) {
		return;
	}

	if(!cJSON_IsArray(fileField)) {
		download(fileField, folderHackontrol);
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(fileField); i++) {
		download(cJSON_GetArrayItem(fileField, i), folderHackontrol);
	}
}
