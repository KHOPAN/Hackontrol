#include "definition.h"

size_t indexOfComma(const char* text, size_t length) {
	for(size_t i = 0; i < length; i++) {
		if(text[i] == ',') {
			return i;
		}
	}

	return -1;
}

EXPORT(DownloadFile) {
	size_t length = strlen(argument);
	size_t index = indexOfComma(argument, length);

	if(index == -1) {
		MessageBoxW(NULL, L"Argument must be in format:\n<url>,<outputPath>", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	char* url = malloc((index + 1) * sizeof(char));

	if(!url) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return;
	}

	size_t outputLength = length - index - 1;
	char* outputFile = malloc((outputLength + 1) * sizeof(char));

	if(!outputFile) {
		free(url);
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return;
	}

	for(size_t i = 0; i < index; i++) {
		url[i] = argument[i];
	}

	for(size_t i = 0; i < outputLength; i++) {
		outputFile[i] = argument[i + index + 1];
	}

	url[index] = 0;
	outputFile[outputLength] = 0;
	CURL* curl = HU_InitializeCURL();
	downloadFileInternal(curl, url, outputFile, FALSE);
	free(url);
	free(outputFile);
}
