#include <stdio.h>
#include <Windows.h>
#include "HackontrolInstaller.h"

void HI_FormatError(unsigned long result, const char* functionName) {
	wchar_t* messageBuffer = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);
	printf("%s error ocurred. Error code: %u Message: %ws", functionName, result, messageBuffer);
}
