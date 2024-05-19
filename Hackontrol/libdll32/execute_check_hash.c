#include "execute.h"
#include <khopanstring.h>
#include <openssl/sha.h>

#define HASH_CHECK(x,y) do{if(cJSON_HasObjectItem(root,x)){cJSON*object=cJSON_GetObjectItem(root,x);if(!cJSON_IsString(object)){goto freeBuffer;}match=y(object,buffer,fileSize.QuadPart);goto freeBuffer;}}while(0)

static BOOL SHA512Check(const cJSON* stringNode, const BYTE* buffer, size_t size);
static BOOL SHA384Check(const cJSON* stringNode, const BYTE* buffer, size_t size);

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

	HASH_CHECK("sha512", SHA512Check);
	HASH_CHECK("sha384", SHA384Check);
freeBuffer:
	LocalFree(buffer);
closeHandle:
	CloseHandle(file);
	return match;
}

static void HexDump(char* output, unsigned long size, unsigned char* input) {
	const char* hexadecimal = "0123456789abcdef";

	for(unsigned char i = 0; i < size; i++) {
		unsigned char index = i * 2;
		output[index] = hexadecimal[(input[i] >> 4) & 0xF];
		output[index + 1] = hexadecimal[input[i] & 0xF];
	}

	output[size * 2] = 0;
}

static BOOL SHA512Check(const cJSON* stringNode, const BYTE* buffer, size_t size) {
	unsigned char hash[SHA512_DIGEST_LENGTH];
	SHA512(buffer, size, hash);
	char output[SHA512_DIGEST_LENGTH * 2 + 1];
	HexDump(output, SHA512_DIGEST_LENGTH, hash);
	MessageBoxA(NULL, KHFormatMessageA("%s\n%s", output, cJSON_GetStringValue(stringNode)), "SHA512 Hash", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return !strcmp(output, cJSON_GetStringValue(stringNode));
}

static BOOL SHA384Check(const cJSON* stringNode, const BYTE* buffer, size_t size) {
	unsigned char hash[SHA384_DIGEST_LENGTH];
	SHA384(buffer, size, hash);
	char output[SHA384_DIGEST_LENGTH * 2 + 1];
	HexDump(output, SHA384_DIGEST_LENGTH, hash);
	MessageBoxA(NULL, KHFormatMessageA("%s\n%s", output, cJSON_GetStringValue(stringNode)), "SHA384 Hash", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return !strcmp(output, cJSON_GetStringValue(stringNode));
}
