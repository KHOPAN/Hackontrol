#include "definition.h"

int indexOf(const char*, char);

EXPORT DownloadFile(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	int index = indexOf(argument, ',');

	if(index == -1) {
		return;
	}

	char* url = malloc((index + 1) * sizeof(char));

	if(url == NULL) {
		return;
	}

	memcpy(url, argument, index);
	url[index] = NULL;
	MessageBoxA(NULL, url, NULL, NULL);
}

int indexOf(const char* text, char character) {
	int length = strlen(text);

	for(int i = 0; i < length; i++) {
		if(text[i] == character) {
			return i;
		}
	}

	return -1;
}
