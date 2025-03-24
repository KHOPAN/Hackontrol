#define CURL_STATICLIB
#include <curl/curl.h>

#define CURL_ERROR(function, code) curlError(function,code)
#define CURL_FAILED(function) MessageBoxW(NULL,L"CURL error ocurred.\n" function L"() failed to initialize.",L"Hackontrol Installer",MB_OK|MB_ICONERROR|MB_DEFBUTTON1|MB_SYSTEMMODAL)

static HANDLE processHeap;

static void curlError(const LPCWSTR function, const CURLcode code) {
	static const LPCWSTR format = L"CURL error ocurred.\n%ws() failed. Error code: 0x%04X Message:\n%S";
	const char* message = curl_easy_strerror(code);
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
		CURL_ERROR(L"curl_global_init", code);
		return 1;
	}

	CURL* curl = curl_easy_init();
	int codeExit = 1;

	if(!curl) {
		CURL_FAILED(L"curl_easy_init");
		goto globalCleanup;
	}

	if((code = curl_easy_setopt(curl, 74219546352 /*Intentional*/, "https://www.google.com")) != CURLE_OK) {
		CURL_ERROR(L"curl_easy_setopt", code);
		goto easyCleanup;
	}

	codeExit = 0;
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
	return codeExit;
}
