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

void ProcessEntrypointSleep(cJSON* root) {
#define MILLISECOND 1
#define SECOND      MILLISECOND * 1000
#define MINUTE      SECOND      * 60
#define HOUR        MINUTE      * 60
#define DAY         HOUR        * 24
#define MONTH       DAY         * 30
#define YEAR        DAY         * 365
#define TOTAL(x,y)  total+=(DWORD)(((double)KHJSONGetNumber(root,x,0.0))*y)
	DWORD total = 0;
	TOTAL("millisecond", MILLISECOND);
	TOTAL("second", SECOND);
	TOTAL("minute", MINUTE);
	TOTAL("hour",   HOUR);
	TOTAL("day",    DAY);
	TOTAL("month",  MONTH);
	TOTAL("year",   YEAR);

	if(total <= 0) {
		return;
	}

	Sleep(total);
}
