#include "khopanjson.h"

cJSON_bool KHJSONGetBoolean(const cJSON* const root, const char* const field, const cJSON_bool defaultValue) {
	if(!root || !field) {
		return defaultValue;
	}

	if(!cJSON_IsObject(root)) {
		return defaultValue;
	}

	if(!cJSON_HasObjectItem(root, field)) {
		return defaultValue;
	}

	cJSON* item = cJSON_GetObjectItem(root, field);

	if(!cJSON_IsBool(item)) {
		return defaultValue;
	}

	return cJSON_IsTrue(item);
}

char* KHJSONGetString(const cJSON* const root, const char* const field, char* const defaultValue) {
	if(!root || !field) {
		return defaultValue;
	}

	if(!cJSON_IsObject(root)) {
		return defaultValue;
	}

	if(!cJSON_HasObjectItem(root, field)) {
		return defaultValue;
	}

	cJSON* item = cJSON_GetObjectItem(root, field);

	if(!cJSON_IsString(item)) {
		return defaultValue;
	}

	return cJSON_GetStringValue(item);
}

cJSON* KHJSONGetObject(const cJSON* const root, const char* const field, cJSON* const defaultValue) {
	if(!root || !field) {
		return defaultValue;
	}

	if(!cJSON_IsObject(root)) {
		return defaultValue;
	}

	if(!cJSON_HasObjectItem(root, field)) {
		return defaultValue;
	}

	cJSON* item = cJSON_GetObjectItem(root, field);

	if(!cJSON_IsObject(item)) {
		return defaultValue;
	}

	return item;
}

cJSON* KHJSONGetArray(const cJSON* const root, const char* const field, cJSON* const defaultValue) {
	if(!root || !field) {
		return defaultValue;
	}

	if(!cJSON_IsObject(root)) {
		return defaultValue;
	}

	if(!cJSON_HasObjectItem(root, field)) {
		return defaultValue;
	}

	cJSON* item = cJSON_GetObjectItem(root, field);

	if(!cJSON_IsArray(item)) {
		return defaultValue;
	}

	return item;
}
