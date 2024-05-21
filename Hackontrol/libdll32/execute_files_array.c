#include "execute.h"
#include <khopanstring.h>

static void processFileEntry(cJSON* root);
static size_t write_file(void* data, size_t size, size_t count, FILE* stream);

void ProcessFilesArray(cJSON* root) {
	if(cJSON_HasObjectItem(root, "file")) {
		processFileEntry(cJSON_GetObjectItem(root, "file"));
	}

	if(!cJSON_HasObjectItem(root, "files")) {
		return;
	}

	cJSON* files = cJSON_GetObjectItem(root, "files");

	if(!cJSON_IsArray(files)) {
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(files); i++) {
		processFileEntry(cJSON_GetArrayItem(files, i));
	}
}

static void processFileEntry(cJSON* root) {
	if(!root || !cJSON_IsObject(root)) {
		return;
	}

	if(!cJSON_HasObjectItem(root, "file")) {
		return;
	}

	cJSON* fileField = cJSON_GetObjectItem(root, "file");

	if(!cJSON_IsString(fileField)) {
		return;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return;
	}

	LPWSTR filePath = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\%S", pathFolderWindows, cJSON_GetStringValue(fileField));
	LocalFree(pathFolderWindows);

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

	CURL* curl = curl_easy_init();

	if(!curl) {
		goto freeFilePath;
	}

	if(curl_easy_setopt(curl, CURLOPT_URL, cJSON_GetStringValue(urlField)) != CURLE_OK) {
		goto easyCleanup;
	}

	if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file) != CURLE_OK) {
		goto easyCleanup;
	}

	FILE* file;

	if(_wfopen_s(&file, filePath, L"wb") || !file) {
		goto easyCleanup;
	}

	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, file) != CURLE_OK) {
		goto closeFile;
	}

	curl_easy_perform(curl);
closeFile:
	fclose(file);
easyCleanup:
	curl_easy_cleanup(curl);
freeFilePath:
	LocalFree(filePath);
}

static size_t write_file(void* data, size_t size, size_t count, FILE* stream) {
	return fwrite(data, size, count, stream);
}
