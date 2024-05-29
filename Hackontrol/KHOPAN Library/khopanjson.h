#pragma once

#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif
cJSON_bool KHJSONGetBoolean(const cJSON* const root, const char* const field, const cJSON_bool defaultValue);
#ifdef __cplusplus
}
#endif
