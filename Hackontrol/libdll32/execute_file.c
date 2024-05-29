#include <khopanstring.h>
#include <hackontrol.h>
#include "execute.h"

#pragma warning(disable: 4996)
#include <openssl/sha.h>
#include <openssl/md5.h>

#define HASH(x,y,z) do{if(cJSON_HasObjectItem(root,x)){cJSON*object=cJSON_GetObjectItem(root,x);if(!cJSON_IsString(object)){goto freeBuffer;}unsigned char hash[z];y(buffer,fileSize.QuadPart,hash);char output[z*2+1];hexDump(output,z,hash);match=!strcmp(output,cJSON_GetStringValue(object));goto freeBuffer;}}while(0)

static void hexDump(char* output, unsigned long size, unsigned char* input);
static BOOL isAbsolute(cJSON* root);

BOOL CheckFileHash(cJSON* root, LPWSTR const filePath) {
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
	HASH("sha1",   SHA1,   SHA_DIGEST_LENGTH);
	HASH("md5",    MD5,    MD5_DIGEST_LENGTH);
freeBuffer:
	LocalFree(buffer);
closeHandle:
	CloseHandle(file);
	return match;
}

LPWSTR GetFilePath(cJSON* root) {
	char* file = cJSON_GetStringValue(cJSON_GetObjectItem(root, "file"));

	if(!file) {
		return NULL;
	}

	if(isAbsolute(root)) {
		return KHFormatMessageW(L"%S", file);
	}

	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		return NULL;
	}

	LPWSTR filePath = KHFormatMessageW(L"%ws\\%S", pathFolderHackontrol, file);
	LocalFree(pathFolderHackontrol);
	return filePath;
}

static void hexDump(char* output, unsigned long size, unsigned char* input) {
	const char* hexadecimal = "0123456789abcdef";

	for(unsigned char i = 0; i < size; i++) {
		unsigned char index = i * 2;
		output[index] = hexadecimal[(input[i] >> 4) & 0xF];
		output[index + 1] = hexadecimal[input[i] & 0xF];
	}

	output[size * 2] = 0;
}

static BOOL isAbsolute(cJSON* root) {
	if(cJSON_HasObjectItem(root, "absolute")) {
		cJSON* item = cJSON_GetObjectItem(root, "absolute");

		if(cJSON_IsBool(item)) {
			return cJSON_IsTrue(item);
		}
	}

	if(cJSON_HasObjectItem(root, "relative")) {
		cJSON* item = cJSON_GetObjectItem(root, "relative");

		if(cJSON_IsBool(item)) {
			return cJSON_IsFalse(item);
		}
	}

	return FALSE;
}
