#include <khopanstring.h>
#include "execute.h"

typedef void (__stdcall* Rundll32Function) (HWND window, HINSTANCE instance, LPSTR argument, int command);

static LPSTR getArgument(cJSON* root);
static void processJoinedDLL(const LPWSTR filePath, const LPSTR functionName, const LPSTR argument);

void ProcessEntrypointDynamicLinkLibrary(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "function")) {
		return;
	}

	cJSON* function = cJSON_GetObjectItem(root, "function");

	if(!cJSON_IsString(function)) {
		return;
	}

	char* functionValue = cJSON_GetStringValue(function);
	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		return;
	}

	LPSTR argument = getArgument(root);
	BOOL join = FALSE;

	if(cJSON_HasObjectItem(root, "join")) {
		cJSON* joinObject = cJSON_GetObjectItem(root, "join");

		if(cJSON_IsBool(joinObject)) {
			join = cJSON_IsTrue(joinObject);
		}
	}

	if(join) {
		processJoinedDLL(filePath, functionValue, argument);
	} else {
		LPWSTR wideFunction = KHFormatMessageW(L"%S", functionValue);

		if(!wideFunction) {
			goto freeFilePath;
		}

		LPWSTR wideArgument = KHFormatMessageW(L"%S", argument);

		if(!wideArgument) {
			LocalFree(wideFunction);
			goto freeFilePath;
		}

		KHWin32StartDynamicLibraryW(filePath, wideFunction, wideArgument);
		LocalFree(wideArgument);
		LocalFree(wideFunction);
	}
freeFilePath:
	LocalFree(filePath);
}

static LPSTR getArgument(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "argument")) {
		return NULL;
	}

	cJSON* argument = cJSON_GetObjectItem(root, "argument");

	if(!cJSON_IsString(argument)) {
		return NULL;
	}

	return cJSON_GetStringValue(argument);
}

static void processJoinedDLL(const LPWSTR filePath, const LPSTR functionName, const LPSTR argument) {
	HMODULE module = LoadLibraryW(filePath);

	if(!module) {
		return;
	}

	Rundll32Function function = (Rundll32Function) GetProcAddress(module, functionName);

	if(!function) {
		return;
	}

	function(NULL, NULL, argument, 0);
}
