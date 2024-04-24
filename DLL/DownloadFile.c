#include "definition.h"

size_t indexOfComma(const char* text, size_t length) {
	for(size_t i = 0; i < length; i++) {
		if(text[i] == ',') {
			return i;
		}
	}

	return -1;
}

static size_t write_data(void* data, size_t size, size_t count, void* stream) {
	return fwrite(data, size, count, (FILE*) stream);
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
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		goto freeURL;
	}

	for(size_t i = 0; i < index; i++) {
		url[i] = argument[i];
	}

	for(size_t i = 0; i < outputLength; i++) {
		outputFile[i] = argument[i + index + 1];
	}

	url[index] = 0;
	outputFile[outputLength] = 0;
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		curlError(code, L"curl_global_init");
		goto freeOutputFile;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		curlError(code, L"curl_easy_init");
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, url);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	FILE* file = NULL;
	errno_t errorCode = fopen_s(&file, outputFile, "wb");

	if(errorCode != 0 || !file) {
		dialogError(errorCode, L"fopen_s");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto closeFile;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_perform");
	}

closeFile:
	fclose(file);
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
freeOutputFile:
	free(outputFile);
freeURL:
	free(url);
}
