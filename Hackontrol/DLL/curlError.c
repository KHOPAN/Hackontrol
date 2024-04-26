#include "errorLog.h"

void curlError(CURLcode errorCode, const wchar_t* functionName) {
	const char* curlMessage = curl_easy_strerror(errorCode);
	const char* formatString = "%ws() error ocurred. Error code: %d Message:\n%s";
	DWORD size = _scprintf(formatString, functionName, errorCode, curlMessage);

	if(size == -1) {
		MessageBoxW(NULL, L"Error while getting the length of the error message", L"Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	size += 1;
	char* displayMessage = malloc(size * sizeof(char));

	if(!displayMessage) {
		MessageBoxW(NULL, L"Out of memory error. Not enough memory for the error message", L"Memory Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	size = sprintf_s(displayMessage, size, formatString, functionName, errorCode, curlMessage);

	if(size == -1) {
		free(displayMessage);
		MessageBoxW(NULL, L"Error while formatting the error message", L"Fatal Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		return;
	}

	MessageBoxA(NULL, displayMessage, "Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	free(displayMessage);
}
