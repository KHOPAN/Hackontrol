#include <stdio.h>
#include <Windows.h>
#include "khopanlibrary.h"

char* allocateString(const char* input) {
	size_t length = strlen(input);
	char* buffer = malloc((length + 1) * sizeof(char));

	if(!buffer) {
		return NULL;
	}
	
	for(size_t i = 0; i < length; i++) {
		buffer[i] = input[i];
	}

	buffer[length] = 0;
	return buffer;
}

char* KHGetWin32ErrorMessageA(unsigned long errorCode, const char* functionName) {
	char* messageBuffer = NULL;
	DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*) &messageBuffer, 0, NULL);

	if(!size) {
		return allocateString("Error while getting the error message text");
	}

	const char* formatString = "%s() error ocurred. Error code: %u Message:\n%s";
	size = _scprintf(formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		return allocateString("Error while getting the length of the error message");
	}

	size += 1;
	char* displayMessage = malloc(size * sizeof(char));

	if(!displayMessage) {
		return allocateString("Out of memory error. Not enough memory for the error message");
	}

	size = sprintf_s(displayMessage, size, formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		free(displayMessage);
		return allocateString("Error while formatting the error message");
	}

	return displayMessage;
}
