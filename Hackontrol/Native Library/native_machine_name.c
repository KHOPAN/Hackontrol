#include <khopanwin32.h>
#include <openssl/sha.h>
#include "native.h"

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

static BOOL GetMachineUUID(const LPSTR outputBuffer);

jstring NativeLibrary_machineName(JNIEnv* environment, jclass nativeLibraryClass) {
	char uuidBuffer[UUID_BUFFER_SIZE + 1];
	srand(GetTickCount64() % 0x100000000);
	
	if(GetMachineUUID(uuidBuffer)) {
		for(unsigned char i = 0; i < UUID_BUFFER_SIZE; i++) {
			uuidBuffer[i] = rand() % 0xFF;
		}
	}

	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(uuidBuffer, UUID_BUFFER_SIZE, hash);
	char hashTextBuffer[SHA_DIGEST_LENGTH * 2 + 1];
	const char* hexadecimal = "0123456789abcdef";

	for(unsigned char i = 0; i < SHA_DIGEST_LENGTH; i++) {
		unsigned char index = i * 2;
		hashTextBuffer[index] = hexadecimal[(hash[i] >> 4) & 0xF];
		hashTextBuffer[index + 1] = hexadecimal[hash[i] & 0xF];
	}

	hashTextBuffer[SHA_DIGEST_LENGTH * 2] = 0;
	return (*environment)->NewStringUTF(environment, hashTextBuffer);
}

static BOOL GetMachineUUID(const LPSTR outputBuffer) {
	UINT size = GetSystemFirmwareTable(RSMB, 0, NULL, 0);

	if(!size) {
		return FALSE;
	}

	RawSMBIOSData* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return FALSE;
	}

	int returnValue = FALSE;

	if(!GetSystemFirmwareTable(RSMB, 0, buffer, size)) {
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
