#include <openssl/sha.h>
#include "exception.h"
#include "Information.h"

#define RSMB 0x52534D42
#define UUID_BUFFER_SIZE 32

typedef struct {
	BYTE  Used20CallingMethod;
	BYTE  SMBIOSMajorVersion;
	BYTE  SMBIOSMinorVersion;
	BYTE  DmiRevision;
	DWORD Length;
	BYTE  SMBIOSTableData[];
} RawSMBIOSData;

typedef struct {
	BYTE type;
	BYTE length;
	WORD handle;
} DMIHeader;

static BOOL getMachineUUID(JNIEnv* const environment, const LPSTR outputBuffer) {
	UINT size = GetSystemFirmwareTable(RSMB, 0, NULL, 0);

	if(!size) {
		HackontrolThrowWin32Error(environment, L"GetSystemFirmwareTable");
		return FALSE;
	}

	RawSMBIOSData* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		return FALSE;
	}

	int returnValue = FALSE;

	if(!GetSystemFirmwareTable(RSMB, 0, buffer, size)) {
		HackontrolThrowWin32Error(environment, L"GetSystemFirmwareTable");
		goto freeBuffer;
	}

	BYTE* pointer = buffer->SMBIOSTableData;

	for(DWORD i = 0; i < buffer->Length; i++) {
		DMIHeader* header = (DMIHeader*) pointer;

		if(header->type == 1) {
			BYTE* uuid = pointer + 0x08;
			BYTE data[16];

			if(buffer->SMBIOSMajorVersion * 0x100 + buffer->SMBIOSMinorVersion >= 0x0206) {
				data[0] = uuid[3];
				data[1] = uuid[2];
				data[2] = uuid[1];
				data[3] = uuid[0];
				data[4] = uuid[5];
				data[5] = uuid[4];
				data[6] = uuid[7];
				data[7] = uuid[6];
			} else {
				for(unsigned char i = 0; i < 8; i++) {
					data[i] = uuid[i];
				}
			}

			for(unsigned char i = 8; i < 16; i++) {
				data[i] = uuid[i];
			}

			const char* hexadecimal = "0123456789ABCDEF";

			for(unsigned char i = 0; i < UUID_BUFFER_SIZE / 2; i++) {
				unsigned char index = i * 2;
				outputBuffer[index] = hexadecimal[(data[i] >> 4) & 0xF];
				outputBuffer[index + 1] = hexadecimal[data[i] & 0xF];
			}

			break;
		}

		pointer += header->length;
		while((*((WORD*) pointer)) != 0) pointer++;
		pointer += 2;
	}

	returnValue = TRUE;
freeBuffer:
	LocalFree(buffer);
	return returnValue;
}

jstring Information_getMachineName(JNIEnv* const environment, const jclass class) {
	char uuid[UUID_BUFFER_SIZE];

	if(!getMachineUUID(environment, uuid)) {
		return NULL;
	}

	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(uuid, UUID_BUFFER_SIZE, hash);
	char text[SHA_DIGEST_LENGTH * 2 + 1];
	const char* hexadecimal = "0123456789abcdef";

	for(unsigned char i = 0; i < SHA_DIGEST_LENGTH; i++) {
		unsigned char index = i * 2;
		text[index] = hexadecimal[(hash[i] >> 4) & 0xF];
		text[index + 1] = hexadecimal[hash[i] & 0xF];
	}

	text[SHA_DIGEST_LENGTH * 2] = 0;
	return (*environment)->NewStringUTF(environment, text);
}
