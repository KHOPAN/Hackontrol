#pragma once

#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif
cJSON_bool KHJSONGetBoolean(const cJSON* const root, const char* const field, const cJSON_bool defaultValue);
char* KHJSONGetString(const cJSON* const root, const char* const field, char* const defaultValue);
cJSON* KHJSONGetObject(const cJSON* const root, const char* const field, cJSON* const defaultValue);
cJSON* KHJSONGetArray(const cJSON* const root, const char* const field, cJSON* const defaultValue);
#ifdef __cplusplus
}
#endif
