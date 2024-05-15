#include <stdio.h>
#include <khopanwin32.h>

#define RSMB 0x52534D42

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

int main(int argc, char** argv) {
	UINT size = GetSystemFirmwareTable(RSMB, 0, NULL, 0);

	if(!size) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetSystemFirmwareTable");
		return 1;
	}

	RawSMBIOSData* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalAlloc");
		return 1;
	}

	int returnValue = 1;

	if(!GetSystemFirmwareTable(RSMB, 0, buffer, size)) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetSystemFirmwareTable");
		goto freeBuffer;
	}

	BYTE* pointer = buffer->SMBIOSTableData;

	for(DWORD i = 0; i < buffer->Length; i++) {
		DMIHeader* header = (DMIHeader*) pointer;

		if(header->type == 1) {
			BYTE* uuid = pointer + 0x08;
			UINT version = buffer->SMBIOSMajorVersion * 0x100 + buffer->SMBIOSMinorVersion;

			if(version >= 0x0206) {
				printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n", uuid[3], uuid[2], uuid[1], uuid[0], uuid[5], uuid[4], uuid[7], uuid[6], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
				return;
			}

			printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n", uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
			break;
		}

		pointer += header->length;
		while((*((WORD*) pointer)) != 0) pointer++;
		pointer += 2;
	}

	returnValue = 0;
freeBuffer:
	if(LocalFree(buffer)) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalFree");
	}

	return returnValue;
}
