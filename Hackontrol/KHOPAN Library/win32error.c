#include <stdio.h>
#include <Windows.h>
#include "khopanerror.h"

char* allocateStringA(const char* input) {
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

wchar_t* allocateStringW(const wchar_t* input) {
	size_t length = wcslen(input);
	wchar_t* buffer = malloc((length + 1) * sizeof(wchar_t));

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
	DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);

	if(!size) {
		return allocateStringA("Error while getting the error message text");
	}

	const char* formatString = "%s() error ocurred. Error code: %u Message:\n%s";
	size = _scprintf(formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		return allocateStringA("Error while getting the length of the error message");
	}

	size += 1;
	char* displayMessage = malloc(size * sizeof(char));

	if(!displayMessage) {
		return allocateStringA("Out of memory error. Not enough memory for the error message");
	}

	size = sprintf_s(displayMessage, size, formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		free(displayMessage);
		return allocateStringA("Error while formatting the error message");
	}

	return displayMessage;
}

wchar_t* KHGetWin32ErrorMessageW(unsigned long errorCode, const wchar_t* functionName) {
	wchar_t* messageBuffer = NULL;
	DWORD size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

	if(!size) {
		return allocateStringW(L"Error while getting the error message text");
	}

	const wchar_t* formatString = L"%ws() error ocurred. Error code: %u Message:\n%ws";
	size = _scwprintf(formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		return allocateStringW(L"Error while getting the length of the error message");
	}

	size += 1;
	wchar_t* displayMessage = malloc(size * sizeof(wchar_t));

	if(!displayMessage) {
		return allocateStringW(L"Out of memory error. Not enough memory for the error message");
	}

	size = swprintf_s(displayMessage, size, formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		free(displayMessage);
		return allocateStringW(L"Error while formatting the error message");
	}

	return displayMessage;
}

void KHWin32DialogErrorW(unsigned long errorCode, const wchar_t* functionName) {
	wchar_t* messageBuffer = NULL;
	DWORD size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

	if(!size) {
		MessageBoxW(NULL, L"Error while getting the error message text", L"Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	const wchar_t* formatString = L"%ws() error ocurred. Error code: %u Message:\n%ws";
	size = _scwprintf(formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		MessageBoxW(NULL, L"Error while getting the length of the error message", L"Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	size += 1;
	wchar_t* displayMessage = malloc(size * sizeof(wchar_t));

	if(!displayMessage) {
		MessageBoxW(NULL, L"Out of memory error. Not enough memory for the error message", L"Memory Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	size = swprintf_s(displayMessage, size, formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		free(displayMessage);
		MessageBoxW(NULL, L"Error while formatting the error message", L"Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	MessageBoxW(NULL, displayMessage, L"Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	free(displayMessage);
}

void KHWin32DialogErrorA(unsigned long errorCode, const char* functionName) {
	char* messageBuffer = NULL;
	DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);

	if(!size) {
		MessageBoxA(NULL, "Error while getting the error message text", "Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	const char* formatString = "%s() error ocurred. Error code: %u Message:\n%s";
	size = _scprintf(formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		MessageBoxA(NULL, "Error while getting the length of the error message", "Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	size += 1;
	char* displayMessage = malloc(size * sizeof(char));

	if(!displayMessage) {
		MessageBoxA(NULL, "Out of memory error. Not enough memory for the error message", "Memory Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	size = sprintf_s(displayMessage, size, formatString, functionName, errorCode, messageBuffer);

	if(size == -1) {
		free(displayMessage);
		MessageBoxA(NULL, "Error while formatting the error message", "Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	MessageBoxA(NULL, displayMessage, "Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	free(displayMessage);
}

void KHWin32ConsoleErrorW(unsigned long errorCode, const wchar_t* functionName) {
	wchar_t* messageBuffer = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);
	printf("%ws() error ocurred. Error code: %u Message:\n%ws\n", functionName, errorCode, messageBuffer);
}

void KHWin32ConsoleErrorA(unsigned long errorCode, const char* functionName) {
	char* messageBuffer = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);
	printf("%s() error ocurred. Error code: %u Message:\n%s\n", functionName, errorCode, messageBuffer);
}
