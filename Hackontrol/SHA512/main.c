#include <stdio.h>
#include <khopanwin32.h>
#include <openssl/sha.h>

#define SHA512_HEXADECIMAL_REPRESENTATION_LENGTH 128

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Error: No input file\n");
		return 1;
	}

	HANDLE file = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32ConsoleErrorW(GetLastError(), L"CreateFileA");
		return 1;
	}
	
	LARGE_INTEGER integer;

	if(!GetFileSizeEx(file, &integer)) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetFileSizeEx");
		return 1;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, integer.QuadPart);

	if(!buffer) {
		KHWin32ConsoleErrorW(GetLastError(), L"LocalAlloc");
		return 1;
	}

	if(!ReadFile(file, buffer, (DWORD) integer.QuadPart, NULL, NULL)) {
		KHWin32ConsoleErrorW(GetLastError(), L"ReadFile");
		return 1;
	}

	if(!CloseHandle(file)) {
		KHWin32ConsoleErrorW(GetLastError(), L"CloseHandle");
		return 1;
	}

	unsigned char hash[SHA512_DIGEST_LENGTH];
	SHA512(buffer, integer.QuadPart, hash);
	char outputBuffer[SHA512_HEXADECIMAL_REPRESENTATION_LENGTH + 1];
	const char* hexadecimal = "0123456789abcdef";

	for(unsigned char i = 0; i < SHA512_DIGEST_LENGTH; i++) {
		unsigned char index = i * 2;
		outputBuffer[index] = hexadecimal[(hash[i] >> 4) & 0xF];
		outputBuffer[index + 1] = hexadecimal[hash[i] & 0xF];
	}

	outputBuffer[SHA512_HEXADECIMAL_REPRESENTATION_LENGTH] = 0;
	printf("%s\n", outputBuffer);
	return 0;
}
