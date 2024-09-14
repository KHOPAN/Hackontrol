#include "hrsp_client_internal.h"

#define ERROR_WIN32(functionName, errorCode) parameter->error.hasError=TRUE;parameter->error.function=functionName;parameter->error.code=errorCode;closesocket(parameter->socket)

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	PBYTE screenshotBuffer = NULL;
	PBYTE qoiBuffer = NULL;
	PBYTE previousBuffer = NULL;
	DWORD returnValue = 1;

	while(parameter->running) {
		if(WaitForSingleObject(parameter->sensitive.mutex, INFINITE) == WAIT_FAILED) {
			ERROR_WIN32(L"WaitForSingleObject", GetLastError());
			goto freeBuffers;
		}

		if(!(parameter->sensitive.flags & 1)) {
			goto releaseMutex;
		}
	releaseMutex:
		if(!ReleaseMutex(parameter->sensitive.mutex)) {
			ERROR_WIN32(L"ReleaseMutex", GetLastError());
			goto freeBuffers;
		}
	}

	/*int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	size_t baseSize = width * height;
	size_t bufferSize = baseSize * 4;
	BYTE* screenshotBuffer = LocalAlloc(LMEM_FIXED, bufferSize);

	if(!screenshotBuffer) {
		return 1;
	}

	BYTE* qoiBuffer = LocalAlloc(LMEM_FIXED, bufferSize);
	BOOL returnValue = 1;

	if(!qoiBuffer) {
		goto freeScreenshotBuffer;
	}

	BYTE* previousBuffer = LocalAlloc(LMEM_FIXED, baseSize * 3);

	if(!previousBuffer) {
		goto freeQOIBuffer;
	}

	while(parameter->running) {
		if(!HRSPClientEncodeCurrentFrame(parameter, width, height, screenshotBuffer, qoiBuffer, previousBuffer)) {
			goto freePreviousBuffer;
		}
	}*/

	returnValue = 0;
freeBuffers:
	if(previousBuffer && LocalFree(previousBuffer)) {
		ERROR_WIN32(L"LocalFree", GetLastError());
		returnValue = FALSE;
	}

	if(qoiBuffer && LocalFree(qoiBuffer)) {
		ERROR_WIN32(L"LocalFree", GetLastError());
		returnValue = FALSE;
	}

	if(screenshotBuffer && LocalFree(screenshotBuffer)) {
		ERROR_WIN32(L"LocalFree", GetLastError());
		return FALSE;
	}

	return returnValue;
}
