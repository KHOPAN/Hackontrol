#include <stdio.h>
#include <Windows.h>
#include <openssl/sha.h>
#include <khopanerror.h>

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Error: No input file\n");
		return 1;
	}

	HANDLE file = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32ConsoleErrorA(GetLastError(), "CreateFileA");
		return 1;
	}
	
	LARGE_INTEGER integer = {0};

	if(!GetFileSizeEx(file, &integer)) {
		KHWin32ConsoleErrorA(GetLastError(), "GetFileSizeEx");
		return 1;
	}

	BYTE* buffer = malloc(integer.QuadPart * sizeof(BYTE));

	if(!buffer) {
		KHWin32ConsoleErrorA(ERROR_OUTOFMEMORY, "malloc");
		return 1;
	}

	if(!ReadFile(file, buffer, (DWORD) integer.QuadPart, NULL, NULL)) {
		KHWin32ConsoleErrorA(GetLastError(), "ReadFile");
		return 1;
	}

	if(!CloseHandle(file)) {
		KHWin32ConsoleErrorA(GetLastError(), "CloseHandle");
		return 1;
	}

	unsigned char hash[SHA512_DIGEST_LENGTH];
	SHA512(buffer, integer.QuadPart, hash);

	for(unsigned char i = 0; i < SHA512_DIGEST_LENGTH; i++) {
		printf("%02x", hash[i]);
	}

	printf("\n");
	return 0;
}
