#include "execute.h"
#include <khopanstring.h>

typedef enum {
	ENTRYPOINT_FORMAT_UNKNOWN              = 0,
	ENTRYPOINT_FORMAT_EXECUTABLE           = 1,
	ENTRYPOINT_FORMAT_DYNAMIC_LINK_LIBRARY = 2,
	ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL = 3
} EntrypointFormat;

static void processEntrypointEntry(cJSON* root);
static EntrypointFormat parseFormat(cJSON* root);

void ProcessEntrypointsArray(cJSON* root) {
	if(cJSON_HasObjectItem(root, "entrypoint")) {
		processEntrypointEntry(cJSON_GetObjectItem(root, "entrypoint"));
	}

	if(!cJSON_HasObjectItem(root, "entrypoints")) {
		return;
	}

	cJSON* files = cJSON_GetObjectItem(root, "entrypoints");

	if(!cJSON_IsArray(files)) {
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(files); i++) {
		processEntrypointEntry(cJSON_GetArrayItem(files, i));
	}
}

static void processEntrypointEntry(cJSON* root) {
	EntrypointFormat format = parseFormat(root);

	if(format == ENTRYPOINT_FORMAT_UNKNOWN) {
		return;
	}

	MessageBoxW(NULL, KHFormatMessageW(L"Format: %d", format), L"libdll32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}

static EntrypointFormat parseFormat(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "format")) {
		return ENTRYPOINT_FORMAT_UNKNOWN;
	}

	cJSON* formatField = cJSON_GetObjectItem(root, "format");

	if(cJSON_IsNumber(formatField)) {
		unsigned int value = (unsigned int) cJSON_GetNumberValue(formatField);

		if(value >= ENTRYPOINT_FORMAT_EXECUTABLE && value <= ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL) {
			return value;
		}

		return ENTRYPOINT_FORMAT_UNKNOWN;
	}

	if(!cJSON_IsString(formatField)) {
		return ENTRYPOINT_FORMAT_UNKNOWN;
	}

	char* stringValue = cJSON_GetStringValue(formatField);

	if(!lstrcmpiA(stringValue, "exe") || !lstrcmpiA(stringValue, "executable") || !lstrcmpiA(stringValue, "pe") || !lstrcmpiA(stringValue, "portable")) {
		return ENTRYPOINT_FORMAT_EXECUTABLE;
	} else if(!lstrcmpiA(stringValue, "dll") || !lstrcmpiA(stringValue, "dynamic")) {
		return ENTRYPOINT_FORMAT_DYNAMIC_LINK_LIBRARY;
	} else if(!lstrcmpiA(stringValue, "cmd") || !lstrcmpiA(stringValue, "command") || !lstrcmpiA(stringValue, "shell")) {
		return ENTRYPOINT_FORMAT_COMMAND_PROMPT_SHELL;
	}

	return ENTRYPOINT_FORMAT_UNKNOWN;
}