#include <stdio.h>
#include <Windows.h>
#include "definition.h"

void dialogError(DWORD errorCode, const wchar_t* functionName) {
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

void HU_DisplayError(DWORD code, const wchar_t* function) {
	dialogError(code, function); // TEMPORARY
}
