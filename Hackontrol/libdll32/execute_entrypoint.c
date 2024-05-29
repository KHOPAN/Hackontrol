#include <khopanstring.h>
#include <khopanjson.h>
#include "execute.h"

void ProcessEntrypointExecutable(cJSON* root) {
	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		return;
	}

	BOOL wait = KHJSONGetBoolean(root, "wait", FALSE);
	BOOL prepend = KHJSONGetBoolean(root, "prepend", FALSE);
	LPSTR argument = KHJSONGetString(root, "argument", NULL);

	if(!argument) {
		if(!prepend) {
			KHWin32StartProcessW(filePath, NULL, wait);
			goto freeFilePath;
		}

		KHWin32StartProcessW(filePath, filePath, wait);
		goto freeFilePath;
	}

	LPWSTR wideArgument = prepend ? KHFormatMessageW(L"%ws %S", filePath, argument) : KHFormatMessageW(L"%S", argument);
	KHWin32StartProcessW(filePath, wideArgument, wait);

	if(wideArgument) {
		LocalFree(wideArgument);
	}
freeFilePath:
	LocalFree(filePath);
}

void ProcessEntrypointDynamicLinkLibrary(cJSON* root) {
	char* function = KHJSONGetString(root, "function", NULL);

	if(!function) {
		return;
	}

	LPWSTR filePath = GetFilePath(root);

	if(!filePath) {
		return;
	}

	LPSTR argument = KHJSONGetString(root, "argument", NULL);

	if(KHJSONGetBoolean(root, "join", FALSE)) {
		KHWin32ExecuteRundll32FunctionW(filePath, function, argument, KHJSONGetBoolean(root, "threaded", FALSE));
		LocalFree(filePath);
		return;
	}

	LPWSTR wideFunction = KHFormatMessageW(L"%S", function);

	if(!wideFunction) {
		LocalFree(filePath);
		return;
	}

	LPWSTR wideArgument = KHFormatMessageW(L"%S", argument);

	if(!wideArgument) {
		LocalFree(wideFunction);
		LocalFree(filePath);
		return;
	}

	KHWin32StartDynamicLibraryW(filePath, wideFunction, wideArgument);
	LocalFree(wideArgument);
	LocalFree(wideFunction);
	LocalFree(filePath);
}

void ProcessEntrypointShell(cJSON* root) {
	LPSTR command = KHJSONGetString(root, "command", NULL);

	if(!command) {
		return;
	}

	KHWin32ExecuteCommandA(command, KHJSONGetBoolean(root, "wait", FALSE));
}
