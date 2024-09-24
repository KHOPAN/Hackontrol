#include <libhackontrol.h>
#include <openssl/sha.h>
#include "execute.h"

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
	DEFINE_HASH("sha1",   SHA1,   SHA_DIGEST_LENGTH);
freeBuffer:
	LocalFree(buffer);
closeHandle:
	CloseHandle(handle);
	return returnValue;
}
