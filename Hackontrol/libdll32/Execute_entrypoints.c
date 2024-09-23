#include <libkhopan.h>
#include "execute.h"

void _stdcall ExecuteEntrypointExecutable(const cJSON* const root, const LPCWSTR folderHackontrol) {
	LPWSTR file = ExecuteGetFile(root, folderHackontrol);

	if(!file) {
		return;
	}

	BOOL block = TRUE;
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
	LPWSTR file = ExecuteGetFile(root, folderHackontrol);

	if(!file) {
		return;
	}

	cJSON* functionField = cJSON_GetObjectItem(root, "function");

	if(!functionField || !cJSON_IsString(functionField)) {
		goto freeFile;
	}

	LPSTR function = cJSON_GetStringValue(functionField);

	if(!function) {
		goto freeFile;
	}
freeFile:
	LocalFree(file);
}

void _stdcall ExecuteEntrypointCommand(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* commandField = cJSON_GetObjectItem(root, "command");

	if(!commandField || !cJSON_IsString(commandField)) {
		return;
	}

	LPSTR command = cJSON_GetStringValue(commandField);

	if(!command) {
		return;
	}

	LPWSTR buffer = KHOPANFormatMessage(L"%S", command);

	if(!buffer) {
		return;
	}

	KHOPANExecuteCommand(buffer, FALSE);
	LocalFree(buffer);
}
