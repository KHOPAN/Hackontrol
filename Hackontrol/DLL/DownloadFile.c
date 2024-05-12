#include <khopanwin32.h>
#include <khopancurl.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

static size_t indexOfComma(const char* text, size_t length);
static size_t write_data(void* data, size_t size, size_t count, FILE* stream);

__declspec(dllexport) void __stdcall DownloadFile(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	size_t length = strlen(argument);
	size_t index = indexOfComma(argument, length);

	if(index == -1) {
		MessageBoxW(NULL, L"Argument must be in format:\n<url>,<outputPath>", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	LPSTR url = LocalAlloc(LMEM_FIXED, (index + 1) * sizeof(CHAR));

	if(!url) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		return;
	}

	size_t outputLength = length - index - 1;
	LPSTR outputFile = LocalAlloc(LMEM_FIXED, (outputLength + 1) * sizeof(CHAR));

	if(!outputFile) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
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
		KHCURLDialogErrorW(code, L"curl_global_init");
		goto freeOutputFile;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		KHCURLDialogErrorW(code, L"curl_easy_init");
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, url);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	FILE* file = NULL;
	errno_t errorCode = fopen_s(&file, outputFile, "wb");

	if(errorCode != 0 || !file) {
		KHWin32DialogErrorW(ERROR_CANNOT_MAKE, L"fopen_s");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto closeFile;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
	}
closeFile:
	fclose(file);
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
freeOutputFile:
	FREE(outputFile);
freeURL:
	FREE(url);
}

static size_t indexOfComma(const char* text, size_t length) {
	for(size_t i = 0; i < length; i++) {
		if(text[i] == ',') {
			return i;
		}
	}

	return -1;
}

static size_t write_data(void* data, size_t size, size_t count, FILE* stream) {
	return fwrite(data, size, count, stream);
}
