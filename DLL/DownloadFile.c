#include "definition.h"

int indexOf(const char*, size_t, char);

EXPORT(DownloadFile) {
	size_t length = strlen(argument);
	int index = indexOf(argument, length, ',');

	if(index == -1) {
		MessageBoxW(NULL, L"Argument must be in format:\n<url>,<outputPath>", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	char* url = malloc((index + 1) * sizeof(char));
	size_t outputLength = length - index - 1;
	char* outputFile = malloc(outputLength * sizeof(char));

	if(url == NULL || outputFile == NULL) {
		HU_DisplayError(ERROR_NOT_ENOUGH_MEMORY, L"malloc()");
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
