#include "execute.h"
#include <khopanstring.h>

static LPWSTR getArgument(cJSON* root, LPWSTR filePath);
static void startProcessAndWait(const LPWSTR filePath, const LPWSTR argument, BOOL wait);

void ProcessEntrypointExecutable(cJSON* root) {
	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		return;
	}

	LPWSTR argument = getArgument(root, filePath);
	BOOL wait = FALSE;

	if(cJSON_HasObjectItem(root, "wait")) {
		cJSON* waitObject = cJSON_GetObjectItem(root, "wait");

		if(cJSON_IsBool(waitObject)) {
			wait = cJSON_IsTrue(waitObject);
		}
	}

	startProcessAndWait(filePath, argument, wait);

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

static void startProcessAndWait(const LPWSTR filePath, const LPWSTR argument, BOOL wait) {
	STARTUPINFOW startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessW(filePath, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInformation, &processInformation)) {
		return;
	}

	if(wait && WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		return;
	}

	if(!CloseHandle(processInformation.hProcess)) {
		return;
	}

	CloseHandle(processInformation.hThread);
}
