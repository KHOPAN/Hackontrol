#include <libhackontrol.h>
#include "execute.h"

LPWSTR ExecuteGetFile(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* fileField = cJSON_GetObjectItem(root, "file");

	if(!fileField || !cJSON_IsString(fileField)) {
		return NULL;
	}

	LPSTR file = cJSON_GetStringValue(fileField);

	if(!file) {
		return NULL;
	}

	BOOL absolute = FALSE;
	cJSON* field = cJSON_GetObjectItem(root, "relative");

	if(field && cJSON_IsBool(field)) {
		absolute = cJSON_IsFalse(field);
	}

	field = cJSON_GetObjectItem(root, "absolute");

	if(field && cJSON_IsBool(field)) {
		absolute = cJSON_IsTrue(field);
	}

	if(absolute) {
		return KHOPANFormatMessage(L"%S", file);
	}

	return KHOPANFormatMessage(L"%ws\\%S", folderHackontrol, file);
}

BOOL ExecuteHashFileCheck(const cJSON* const root, const LPCWSTR file) {
	return FALSE;
}
