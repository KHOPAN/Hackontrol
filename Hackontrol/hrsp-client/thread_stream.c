#include "hrsp_client_internal.h"

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	int width = GetSystemMetrics(SM_CXSCREEN);
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
	}

	returnValue = 0;
freePreviousBuffer:
	LocalFree(previousBuffer);
freeQOIBuffer:
	LocalFree(qoiBuffer);
freeScreenshotBuffer:
	LocalFree(screenshotBuffer);
	return returnValue;
}
