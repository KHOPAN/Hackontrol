#include "packet.h"
#include <khopanstring.h>
#include <khopanjava.h>
#include <khopandatastream.h>
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
	size_t bufferSize = width * height * 4;
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

	if(!GetDIBits(memoryContext, bitmap, 0, height, buffer, (LPBITMAPINFO) &header, DIB_RGB_COLORS)) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"GetDIBits");
		LocalFree(buffer);
		goto cleanup;
	}

	BYTE* encodedResult = LocalAlloc(LMEM_FIXED, bufferSize * 2);

	if(!encodedResult) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		LocalFree(buffer);
		goto cleanup;
	}

	size_t encodedPointer = 0;
	encodedResult[encodedPointer++] = (width >> 24) & 0xFF;
	encodedResult[encodedPointer++] = (width >> 16) & 0xFF;
	encodedResult[encodedPointer++] = (width >> 8) & 0xFF;
	encodedResult[encodedPointer++] = width & 0xFF;
	encodedResult[encodedPointer++] = (height >> 24) & 0xFF;
	encodedResult[encodedPointer++] = (height >> 16) & 0xFF;
	encodedResult[encodedPointer++] = (height >> 8) & 0xFF;
	encodedResult[encodedPointer++] = height & 0xFF;
	BYTE previousRed = 0;
	BYTE previousGreen = 0;
	BYTE previousBlue = 0;
	BYTE runLength = 0;
	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	memset(seenRed, 0, sizeof(seenRed));
	memset(seenGreen, 0, sizeof(seenGreen));
	memset(seenBlue, 0, sizeof(seenBlue));
	BYTE run = 0;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int position = ((height - y - 1) * width + x) * 4;
			BYTE red = buffer[position + 2];
			BYTE green = buffer[position + 1];
			BYTE blue = buffer[position];

			if(red == previousRed && green == previousGreen && blue == previousBlue) {
				run++;

				if(run == 62) {
					encodedResult[encodedPointer++] = 0b11000000 | ((run - 1) & 0b111111);
					run = 0;
				}

				continue;
			}

			if(run > 0) {
				encodedResult[encodedPointer++] = 0b11000000 | ((run - 1) & 0b111111);
				run = 0;
			}

			BYTE indexPosition = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;

			if(red == seenRed[indexPosition] && green == seenGreen[indexPosition] && blue == seenBlue[indexPosition]) {
				encodedResult[encodedPointer++] = indexPosition & 0b111111;
				previousRed = red;
				previousGreen = green;
				previousBlue = blue;
				continue;
			}

			seenRed[indexPosition] = red;
			seenGreen[indexPosition] = green;
			seenBlue[indexPosition] = blue;
			BYTE differenceRed = red - previousRed;
			BYTE differenceGreen = green - previousGreen;
			BYTE differenceBlue = blue - previousBlue;

			if(differenceRed > -3 && differenceRed < 2 && differenceGreen > -3 && differenceGreen < 2 && differenceBlue > -3 && differenceBlue < 2) {
				encodedResult[encodedPointer++] = 0b0100000 | (((differenceRed + 2) & 0b11) << 4) | (((differenceGreen + 2) & 0b11) << 2) | ((differenceBlue + 2) & 0b11);
			} else {
				char relativeRed = differenceRed - differenceGreen;
				char relativeBlue = differenceBlue - differenceGreen;

				if(relativeRed > -9 && relativeRed < 8 && differenceGreen > -33 && differenceGreen < 32 && relativeBlue > -9 && relativeBlue < 8) {
					encodedResult[encodedPointer++] = 0b10000000 | ((differenceGreen + 32) & 0b111111);
					encodedResult[encodedPointer++] = (((relativeRed + 8) & 0b1111) << 4) | ((relativeBlue + 8) & 0b1111);
				} else {
					encodedResult[encodedPointer++] = 0b11111110;
					encodedResult[encodedPointer++] = red;
					encodedResult[encodedPointer++] = green;
					encodedResult[encodedPointer++] = blue;
				}
			}

			previousRed = red;
			previousGreen = green;
			previousBlue = blue;
		}
	}

	if(run > 0) {
		encodedResult[encodedPointer++] = 0b11000000 | ((run - 1) & 0b111111);
	}

	LocalFree(buffer);
	PACKET packet;
	packet.size = (long) encodedPointer;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = encodedResult;
	BOOL result = SendPacket(clientSocket, &packet);
	LocalFree(encodedResult);

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
