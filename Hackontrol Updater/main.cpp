#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <string>
#include <Windows.h>
#include "HackontrolUpdater.h"

#define FILE_NAME L"hksct32.exe"

int executeProgram(wchar_t*);

int main(int argc, char** argv) {
	wchar_t* filePath = HU_GetSystemDirectory(FILE_NAME);
	CURL* curl = HU_InitializeCURL();
	const char* versionFile = HU_GetVersionFile(curl);
	rapidjson::Document document;
	document.Parse(versionFile);

	if(!document.HasMember("hash")) {
		MessageBoxW(NULL, L"JSON field 'hash' not found", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return -5;
	}

	rapidjson::Value& hashElement = document["hash"];

	if(!hashElement.IsString()) {
		MessageBoxW(NULL, L"JSON field 'hash' is not string", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return -6;
	}

	const char* sourceHash = hashElement.GetString();
	HANDLE file = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		HU_DisplayError(GetLastError(), L"CreateFileW()");
		return -7;
	}
	
	if(HU_IsFileExists(filePath)) {
		LARGE_INTEGER integer;

		if(GetFileSizeEx(file, &integer) == NULL) {
			HU_DisplayError(GetLastError(), L"GetFileSizeEx()");
			return -8;
		}

		BYTE* dataBuffer = static_cast<BYTE*>(malloc(integer.QuadPart));

		if(dataBuffer == NULL) {
			HU_DisplayError(ERROR_NOT_ENOUGH_MEMORY, L"malloc()");
			return -9;
		}

		DWORD bytesRead = 0;

		if(ReadFile(file, dataBuffer, static_cast<DWORD>(integer.QuadPart), &bytesRead, NULL) == NULL) {
			HU_DisplayError(GetLastError(), L"ReadFile()");
			return -10;
		}

		const char* hash = HU_Hash(dataBuffer, bytesRead);
		free(dataBuffer);

		if(strcmp(hash, sourceHash) == 0) {
			free(const_cast<char*>(hash));
			
			if(CloseHandle(file) == NULL) {
				HU_DisplayError(GetLastError(), L"CloseHandle()");
				return -11;
			}

			return executeProgram(filePath);
		}

		free(const_cast<char*>(hash));
	}

	if(CloseHandle(file) == NULL) {
		HU_DisplayError(GetLastError(), L"CloseHandle()");
		return -12;
	}

	if(!document.HasMember("download")) {
		MessageBoxW(NULL, L"JSON field 'download' not found", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return -13;
	}

	rapidjson::Value& downloadElement = document["download"];

	if(!downloadElement.IsString()) {
		MessageBoxW(NULL, L"JSON field 'download' is not string", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return -14;
	}

	const char* downloadURL = downloadElement.GetString();
	HU_DownloadFile(curl, downloadURL, filePath);
	return executeProgram(filePath);
}

int executeProgram(wchar_t* filePath) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION processInformtion = {0};

	if(CreateProcessW(filePath, NULL, NULL, NULL, TRUE, NULL, NULL, NULL, &startupInformation, &processInformtion) == NULL) {
		free(filePath);
		HU_DisplayError(GetLastError(), L"CreateProcessW()");
		return -17;
	}

	if(CloseHandle(processInformtion.hProcess) == NULL) {
		HU_DisplayError(GetLastError(), L"CloseHandle()");
		return -18;
	}

	if(CloseHandle(processInformtion.hThread) == NULL) {
		HU_DisplayError(GetLastError(), L"CloseHandle()");
		return -19;
	}

	return 0;
}
