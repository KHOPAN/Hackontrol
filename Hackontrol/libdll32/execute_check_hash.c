#include "execute.h"
#include <openssl/sha.h>
#include <khopanstring.h>

static BOOL SHA512Check(const cJSON* stringNode, const BYTE* buffer, size_t size);

BOOL CheckFileHash(cJSON* root, LPWSTR filePath) {
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

	if(cJSON_HasObjectItem(root, "sha512")) {
		match = SHA512Check(cJSON_GetObjectItem(root, "sha512"), buffer, fileSize.QuadPart);
		goto freeBuffer;
	}

freeBuffer:
	LocalFree(buffer);
closeHandle:
	CloseHandle(file);
	return match;
}

static BOOL SHA512Check(const cJSON* stringNode, const BYTE* buffer, size_t size) {
	if(!cJSON_IsString(stringNode)) {
		return FALSE;
	}

	char* string = cJSON_GetStringValue(stringNode);
	MessageBoxA(NULL, string, "libdll32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return TRUE;
}
