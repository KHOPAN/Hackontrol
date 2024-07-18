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
#define APPEND(z) encodedResult[encodedPointer++]=z
	APPEND((width >> 24) & 0xFF);
	APPEND((width >> 16) & 0xFF);
	APPEND((width >> 8) & 0xFF);
	APPEND(width & 0xFF);
	APPEND((height >> 24) & 0xFF);
	APPEND((height >> 16) & 0xFF);
	APPEND((height >> 8) & 0xFF);
	APPEND(height & 0xFF);

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int position = (y * width + x) * 4;
			BYTE red = buffer[position + 2];
			BYTE green = buffer[position + 1];
			BYTE blue = buffer[position];
			BYTE indexPosition = (red * 3 + green * 5 + blue * 7) % 64;

			if(previousRed == red && previousGreen == green && previousBlue == blue) {
				runLength++;

				if(runLength < 63) {
					continue;
				}

				APPEND(0b11000000 | ((runLength - 1) & 0b111111));
				runLength = 0;
				continue;
			} else if(runLength) {
				APPEND(0b11000000 | ((runLength - 1) & 0b111111));
			}

			if(seenRed[indexPosition] == red && seenGreen[indexPosition] == green && seenBlue[indexPosition] == blue) {
				APPEND(indexPosition & 0b111111);
				continue;
			}

			seenRed[indexPosition] = red;
			seenGreen[indexPosition] = green;
			seenBlue[indexPosition] = blue;
			char differenceRed = red - previousRed;
			char differenceGreen = green - previousGreen;
			char differenceBlue = blue - previousBlue;

			if(differenceRed >= -2 && differenceRed <= 1 && differenceGreen >= -2 && differenceGreen <= 1 && differenceBlue >= -2 && differenceBlue <= 1) {
				APPEND(0b0100000 | (((differenceRed + 2) & 0b11) << 4) | (((differenceGreen + 2) & 0b11) << 2) | ((differenceBlue + 2) & 0b11));
				continue;
			}

			differenceRed -= differenceGreen;
			differenceBlue -= differenceGreen;

			if(differenceRed >= -8 && differenceRed <= 7 && differenceGreen >= -32 && differenceGreen <= 31 && differenceBlue >= -8 && differenceBlue <= 7) {
				APPEND(0b1000000 | ((differenceGreen + 32) & 0b111111));
				APPEND((((differenceRed + 8) & 0b1111) << 4) | ((differenceBlue + 8) & 0b1111));
				continue;
			}

			APPEND(0b11111110);
			APPEND(red);
			APPEND(green);
			APPEND(blue);
		}
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
