#define CURL_STATICLIB
#include <curl/curl.h>

static const LPCWSTR programName = L"Hackontrol Installer";

static HANDLE processHeap;

int main(int argc, char** argv) {
	processHeap = GetProcessHeap();

	if(!processHeap) {
		MessageBoxW(NULL, L"GetProcessHeap() failed.", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	}

	return 0;
}

/*typedef struct {
	PBYTE data;
	size_t size;
} DATABUFFER, *PDATABUFFER;

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

static size_t curlWriteCallback(const char* const data, size_t size, size_t count, const PDATABUFFER pointer) {
	size *= count;
	PBYTE buffer = HeapAlloc(processHeap, 0, pointer->size + size);

	if(!buffer) {
		return 0;
	}

	if(pointer->data) {
		for(count = 0; count < pointer->size; count++) {
			buffer[count] = pointer->data[count];
		}

		HeapFree(processHeap, 0, pointer->data);
	}

	for(count = 0; count < size; count++) {
		buffer[count + pointer->size] = data[count];
	}

	pointer->data = buffer;
	pointer->size += size;
	return size;
}

int main(int argc, char** argv) {
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

	for(codeExit = 0; codeExit <= CURL_ERROR_SIZE; codeExit++) {
		errorBuffer[codeExit] = 0;
	}

	codeExit = 1;

	if((code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer)) != CURLE_OK) {
		curlError(L"curl_easy_setopt", code, NULL);
		goto easyCleanup;
	}

	if((code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0)) != CURLE_OK) {
		curlError(L"curl_easy_setopt", code, NULL);
		goto easyCleanup;
	}

	if((code = curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com")) != CURLE_OK) {
		curlError(L"curl_easy_setopt", code, NULL);
		goto easyCleanup;
	}

	DATABUFFER buffer = {0};

	if((code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer)) != CURLE_OK) {
		curlError(L"curl_easy_setopt", code, NULL);
		goto easyCleanup;
	}

	if((code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback)) != CURLE_OK) {
		curlError(L"curl_easy_setopt", code, NULL);
		goto easyCleanup;
	}

	if((code = curl_easy_perform(curl)) != CURLE_OK) {
		if(buffer.data) {
			HeapFree(processHeap, 0, buffer.data);
		}

		errorBuffer[CURL_ERROR_SIZE] = 0;
		curlError(L"curl_easy_perform", code, errorBuffer);
		goto easyCleanup;
	}

	if(buffer.data) {
		printf("%.*s\n", (int) buffer.size, buffer.data);
		HeapFree(processHeap, 0, buffer.data);
	}

	codeExit = 0;
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
	return codeExit;
}*/
