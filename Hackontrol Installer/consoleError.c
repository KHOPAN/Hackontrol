#include "installer.h"

void consoleError(unsigned long result, const wchar_t* functionName) {
	wchar_t* messageBuffer = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);
	printf("%ws() error ocurred. Error code: %u Message: %ws", functionName, result, messageBuffer);
}
