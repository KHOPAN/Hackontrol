#include "execute.h"
#include <khopanstring.h>

static LPWSTR getArgument(cJSON* root, LPWSTR filePath);

void ProcessEntrypointExecutable(cJSON* root) {
	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		return;
	}

	LPWSTR argument = getArgument(root, filePath);
	KHWin32StartProcessW(filePath, argument);

	if(argument) {
		LocalFree(argument);
	}

	LocalFree(filePath);
}

static LPWSTR getArgument(cJSON* root, LPWSTR filePath) {
	BOOL prepend = FALSE;

	if(cJSON_HasObjectItem(root, "prepend")) {
		cJSON* prependObject = cJSON_GetObjectItem(root, "prepend");

		if(cJSON_IsBool(prependObject)) {
			prepend = cJSON_IsTrue(prependObject);
		}
	}

	if(!cJSON_HasObjectItem(root, "argument")) {
		return prepend ? filePath : NULL;
	}

	cJSON* argument = cJSON_GetObjectItem(root, "argument");

	if(!cJSON_IsString(argument)) {
		return prepend ? filePath : NULL;
	}

	char* argumentValue = cJSON_GetStringValue(argument);

	if(!prepend) {
		return KHFormatMessageW(L"%S", argumentValue);
	}

	return KHFormatMessageW(L"%ws %S", filePath, argumentValue);
}
