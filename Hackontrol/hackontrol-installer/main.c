#define CURL_STATICLIB
#include <curl/curl.h>

static HANDLE processHeap;

static void curlError(const LPCWSTR function, const CURLcode code, const LPCSTR errorBuffer) {
	static const LPCWSTR format = L"CURL error ocurred.\n%ws() failed. Error code: 0x%04X Message:\n%S";
	const char* message = errorBuffer ? errorBuffer : curl_easy_strerror(code);
	int length = _scwprintf(format, function, code, message);

	if(length < 1) {
		return;
	}

	LPWSTR buffer = HeapAlloc(processHeap, 0, sizeof(WCHAR) * (length + 1));

	if(!buffer) {
		return;
	}

	if((length = swprintf_s(buffer, length + 1, format, function, code, message)) < 1) {
		HeapFree(processHeap, 0, buffer);
		return;
	}

	buffer[length] = 0;
	MessageBoxW(NULL, buffer, L"Hackontrol Installer", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	HeapFree(processHeap, 0, buffer);
}

int main(int argc, char** argv) {
	processHeap = GetProcessHeap();
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		curlError(L"curl_global_init", code, NULL);
		return 1;
	}

	CURL* curl = curl_easy_init();
	int codeExit = 1;

	if(!curl) {
		MessageBoxW(NULL, L"CURL error ocurred.\ncurl_easy_init() failed to initialize.", L"Hackontrol Installer", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto globalCleanup;
	}

	char errorBuffer[CURL_ERROR_SIZE + 1];

	if((code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer)) != CURLE_OK) {
		curlError(L"curl_easy_setopt", code, NULL);
		goto easyCleanup;
	}

	/*if((code = curl_easy_setopt(curl, CURLOPT_TIMEVALUE, -1)) != CURLE_OK) {
		printf("Error: %s\n", errorBuffer);
		curlError(L"curl_easy_setopt", code);
		goto easyCleanup;
	}*/

	if((code = curl_easy_perform(curl)) != CURLE_OK) {
		errorBuffer[CURL_ERROR_SIZE] = 0;
		curlError(L"curl_easy_perform", code, errorBuffer);
		goto easyCleanup;
	}

	codeExit = 0;
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
	return codeExit;
}
