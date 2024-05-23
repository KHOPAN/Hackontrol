#include "execute.h"
#include <khopanstring.h>

typedef void (__stdcall* ExecuteFunction) (HWND window, HINSTANCE instance, LPSTR argument, int command);

static LPWSTR getArgument(cJSON* root);
static void processJoinedDLL(const LPWSTR filePath, const LPSTR functionName, const LPWSTR argument);

void ProcessEntrypointDynamicLinkLibrary(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "function")) {
		return;
	}

	cJSON* function = cJSON_GetObjectItem(root, "function");

	if(!cJSON_IsString(function)) {
		return;
	}

	char* functionRaw = cJSON_GetStringValue(function);
	LPWSTR functionName = KHFormatMessageW(L"%S", functionRaw);

	if(!functionName) {
		return;
	}

	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		LocalFree(functionName);
		return;
	}

	LPWSTR argument = getArgument(root);
	BOOL join = FALSE;

	if(cJSON_HasObjectItem(root, "join")) {
		cJSON* joinObject = cJSON_GetObjectItem(root, "join");

		if(cJSON_IsBool(joinObject)) {
			join = cJSON_IsTrue(joinObject);
		}
	}

	if(join) {
		processJoinedDLL(filePath, functionRaw, argument);
	} else {
		KHWin32StartDynamicLibraryW(filePath, functionName, argument);
	}

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

static void processJoinedDLL(const LPWSTR filePath, const LPSTR functionName, const LPWSTR argument) {
	LPSTR newArgument = KHFormatMessageA("%ws", argument);

	if(!newArgument) {
		return;
	}

	HMODULE module = LoadLibraryW(filePath);

	if(!module) {
		LocalFree(newArgument);
		return;
	}

	ExecuteFunction function = (ExecuteFunction) GetProcAddress(module, functionName);

	if(!function) {
		LocalFree(newArgument);
		return;
	}

	function(NULL, NULL, newArgument, 0);
	LocalFree(newArgument);
}
