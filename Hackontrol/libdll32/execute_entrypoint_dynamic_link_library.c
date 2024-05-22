#include "execute.h"
#include <khopanstring.h>

static LPWSTR getArgument(cJSON* root);

void ProcessEntrypointDynamicLinkLibrary(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "function")) {
		return;
	}

	cJSON* function = cJSON_GetObjectItem(root, "function");

	if(!cJSON_IsString(function)) {
		return;
	}

	LPWSTR functionName = KHFormatMessageW(L"%S", cJSON_GetStringValue(function));

	if(!functionName) {
		return;
	}

	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		LocalFree(functionName);
		return;
	}

	LPWSTR argument = getArgument(root);
	StartRundll32(filePath, functionName, argument);

	if(argument) {
		LocalFree(argument);
	}

	LocalFree(filePath);
	LocalFree(functionName);
}

static LPWSTR getArgument(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "argument")) {
		return NULL;
	}

	cJSON* argument = cJSON_GetObjectItem(root, "argument");

	if(!cJSON_IsString(argument)) {
		return NULL;
	}

	return KHFormatMessageW(L"%S", cJSON_GetStringValue(argument));
}
