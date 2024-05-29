#include <stdio.h>
#include <khopanwin32.h>
#include <cJSON.h>

int main(int argc, char** argv) {
	cJSON* root = cJSON_Parse("{\"test\":false}");

	if(!root) {
		printf("Parse error\n");
		return 1;
	}

	printf("Bool: %d\n", cJSON_IsFalse(cJSON_GetObjectItem(root, "test")));
	cJSON_Delete(root);
	return 0;
}
