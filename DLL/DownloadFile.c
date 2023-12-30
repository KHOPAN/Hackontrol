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

	memcpy(url, argument, index);
	memcpy(outputFile, argument + index + 1, outputLength);
	url[index] = 0;
	MessageBoxA(NULL, url, outputFile, MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}

int indexOf(const char* text, size_t length, char character) {
	for(int i = 0; i < length; i++) {
		if(text[i] == character) {
			return i;
		}
	}

	return -1;
}
