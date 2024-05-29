#include <stdio.h>
#include <khopanwin32.h>
#include <khopanjson.h>

int main(int argc, char** argv) {
	cJSON* root = cJSON_Parse("{\"test\":false}");

	if(!root) {
		printf("Parse error\n");
		return 1;
	}

	printf("Bool: %s\n", KHJSONGetBoolean(root, "test", TRUE) ? "true" : "false");
	cJSON_Delete(root);
	return 0;
}
