#include "definition.h"

int indexOf(const char*, size_t, char);

EXPORT DownloadFile(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	size_t length = strlen(argument);
	int index = indexOf(argument, length, ',');

	if(index == -1) {
		return;
	}

	char* url = malloc((index + 1) * sizeof(char));
	size_t outputLength = length - index - 1;
	char* outputFile = malloc(outputLength * sizeof(char));

	if(url == NULL || outputFile == NULL) {
		return;
	}

	CURL* curl = HU_InitializeCURL();
	memcpy(url, argument, index);
	memcpy(outputFile, argument + index + 1, outputLength);
	url[index] = 0;
	HU_DownloadFile(curl, url, outputFile, FALSE);
}

int indexOf(const char* text, size_t length, char character) {
	for(int i = 0; i < length; i++) {
		if(text[i] == character) {
			return i;
		}
	}

	return -1;
}
