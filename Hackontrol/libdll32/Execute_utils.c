#include <libhackontrol.h>
#include "execute.h"

LPWSTR ExecuteGetFile(const cJSON* const root) {
	cJSON* fileField = cJSON_GetObjectItem(root, "file");

	if(!fileField || !cJSON_IsString(fileField)) {
		return NULL;
	}

	LPSTR file = cJSON_GetStringValue(fileField);

	if(!file) {
		return NULL;
	}

	BOOL absolute = FALSE;
	cJSON* relativeField = cJSON_GetObjectItem(root, "relative");

	if(relativeField && cJSON_IsBool(relativeField)) {
		absolute = cJSON_IsFalse(relativeField);
	}

	cJSON* absoluteField = cJSON_GetObjectItem(root, "absolute");

	if(absoluteField && cJSON_IsBool(absoluteField)) {
		absolute = cJSON_IsTrue(absoluteField);
	}

	if(absolute) {
		return KHOPANFormatMessage(L"%S", file);
	}

	LPWSTR folderHackontrol = HackontrolGetHomeDirectory();

	if(!folderHackontrol) {
		return NULL;
	}

	if(!HackontrolCreateDirectory(folderHackontrol)) {
		LocalFree(folderHackontrol);
		return NULL;
	}

	LPWSTR path = KHOPANFormatMessage(L"%ws\\%S", folderHackontrol, file);
	LocalFree(folderHackontrol);
	return path;
}

BOOL HashFileCheck(const cJSON* const root, const LPCWSTR file) {
	return FALSE;
}
