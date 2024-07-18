#include "packet.h"
#include <khopanstring.h>
#include <khopanjava.h>
#include "screenshot.h"
#include "exception.h"

BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket, LodePNGState* state) {
	HDC context = GetDC(NULL);
	int width = GetDeviceCaps(context, HORZRES);
	int height = GetDeviceCaps(context, VERTRES);
	HDC memoryContext = CreateCompatibleDC(context);
	HBITMAP bitmap = CreateCompatibleBitmap(context, width, height);
	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
	BitBlt(memoryContext, 0, 0, width, height, context, 0, 0, SRCCOPY);
	bitmap = SelectObject(memoryContext, oldBitmap);
	BYTE* buffer = LocalAlloc(LMEM_FIXED, width * height * 4);
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

	BYTE* pngImage;
	size_t pngSize;
	unsigned int status = lodepng_encode(&pngImage, &pngSize, buffer, width, height, state);
	LocalFree(buffer);

	if(status) {
		LPWSTR message = KHFormatMessageW(L"lodepng_encode() %S (Error code: %u)", lodepng_error_text(status), status);

		if(!message) {
			goto cleanup;
		}

		KHJavaThrowInternalErrorW(environment, message);
		LocalFree(message);
		goto cleanup;
	}

	PACKET packet;
	packet.size = (long) pngSize;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = pngImage;
	BOOL result = SendPacket(clientSocket, &packet);
	free(pngImage);

	if(!result) {
		HackontrolThrowWin32Error(environment, L"SendPacket");
		goto cleanup;
	}

	returnValue = TRUE;
cleanup:
	DeleteObject(bitmap);
	DeleteDC(memoryContext);
	ReleaseDC(NULL, context);
	return returnValue;
}
