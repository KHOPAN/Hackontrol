#include "execute.h"

static EXECUTEENTRYPOINT entrypoints[] = {
	{"executable", "exe", ExecuteEntrypointExecutable}
};

static void entrypoint(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* formatField = cJSON_GetObjectItem(root, "format");

	if(!formatField) {
		return;
	}

	UINT index;

	if(cJSON_IsNumber(formatField)) {
		index = (UINT) cJSON_GetNumberValue(formatField);

		if(index >= 0 && index < sizeof(entrypoints) / sizeof(entrypoints[0])) {
			entrypoints[index].function(root, folderHackontrol);
			return;
		}
	}

	if(!cJSON_IsString(formatField)) {
		return;
	}

	LPSTR format = cJSON_GetStringValue(formatField);

	if(!format) {
		return;
	}

	for(index = 0; index < sizeof(entrypoints) / sizeof(entrypoints[0]); index++) {
		if(!lstrcmpiA(format, entrypoints[index].fullName) || !lstrcmpiA(format, entrypoints[index].shortName)) {
			entrypoints[index].function(root, folderHackontrol);
			return;
		}
	}
}

void ExecuteExecute(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* entrypointField = cJSON_GetObjectItem(root, "entrypoint");

	if(!entrypointField) {
		return;
	}

	if(!cJSON_IsArray(entrypointField)) {
		entrypoint(entrypointField, folderHackontrol);
		return;
	}

	for(int i = 0; i < cJSON_GetArraySize(entrypointField); i++) {
		entrypoint(cJSON_GetArrayItem(entrypointField, i), folderHackontrol);
	}
}
