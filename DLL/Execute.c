//#include <rapidjson/document.h>
//#include <rapidjson/reader.h>
//#include <string>
#include "definition.h"

#define FILE_NAME_PATH L"System32\\ctrl32.dll"

//void executeProgram();

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
	MessageBoxW(NULL, fileNameBuffer, L"Execute", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	//wchar_t* filePath = HU_GetSystemDirectory(FILE_NAME);
	/*CURL* curl = HU_InitializeCURL();
	const char* versionFile = HU_GetVersionFile(curl);
	rapidjson::Document document;
	document.Parse(versionFile);

	if(!document.HasMember("hash")) {
		MessageBoxW(NULL, L"JSON field 'hash' not found", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	rapidjson::Value& hashElement = document["hash"];

	if(!hashElement.IsString()) {
		MessageBoxW(NULL, L"JSON field 'hash' is not string", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	const char* sourceHash = hashElement.GetString();
	HANDLE file = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		HU_DisplayError(GetLastError(), L"CreateFileW()");
		return;
	}

	if(HU_IsFileExists(filePath)) {
		LARGE_INTEGER integer;

		if(GetFileSizeEx(file, &integer) == NULL) {
			HU_DisplayError(GetLastError(), L"GetFileSizeEx()");
			return;
		}

		BYTE* dataBuffer = static_cast<BYTE*>(malloc(integer.QuadPart));

		if(dataBuffer == NULL) {
			HU_DisplayError(ERROR_NOT_ENOUGH_MEMORY, L"malloc()");
			return;
		}

		DWORD bytesRead = 0;

		if(ReadFile(file, dataBuffer, static_cast<DWORD>(integer.QuadPart), &bytesRead, NULL) == NULL) {
			HU_DisplayError(GetLastError(), L"ReadFile()");
			return;
		}

		const char* hash = HU_Hash(dataBuffer, bytesRead);
		free(dataBuffer);

		if(strcmp(hash, sourceHash) == 0) {
			free(const_cast<char*>(hash));

			if(CloseHandle(file) == NULL) {
				HU_DisplayError(GetLastError(), L"CloseHandle()");
				return;
			}

			free(filePath);
			executeProgram();
			return;
		}

		free(const_cast<char*>(hash));
	}

	if(CloseHandle(file) == NULL) {
		HU_DisplayError(GetLastError(), L"CloseHandle()");
		return;
	}

	if(!document.HasMember("download")) {
		MessageBoxW(NULL, L"JSON field 'download' not found", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	rapidjson::Value& downloadElement = document["download"];

	if(!downloadElement.IsString()) {
		MessageBoxW(NULL, L"JSON field 'download' is not string", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return;
	}

	const char* downloadURL = downloadElement.GetString();
	downloadFileInternal(curl, downloadURL, filePath, TRUE);
	free(filePath);
	executeProgram();*/
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
