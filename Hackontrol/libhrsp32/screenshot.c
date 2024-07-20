#include "packet.h"
#include <khopanstring.h>
#include <khopanjava.h>
#include <khopandatastream.h>
#include "screenshot.h"
#include "exception.h"

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000

static void drawCursor(const HDC context) {
	CURSORINFO cursorInformation;
	cursorInformation.cbSize = sizeof(CURSORINFO);

	if(!GetCursorInfo(&cursorInformation) || cursorInformation.flags != CURSOR_SHOWING) {
		return;
	}

	HICON icon = CopyIcon(cursorInformation.hCursor);

	if(!icon) {
		return;
	}

	ICONINFO iconInformation;

	if(!GetIconInfo(icon, &iconInformation)) {
		goto destroyIcon;
	}

	if(!iconInformation.hbmColor) {
		goto deleteIconBitmap;
	}

	BITMAP bitmap;

	if(!GetObjectW(iconInformation.hbmColor, sizeof(bitmap), &bitmap)) {
		goto deleteIconBitmap;
	}

	if(!DrawIconEx(context, cursorInformation.ptScreenPos.x - iconInformation.xHotspot, cursorInformation.ptScreenPos.y - iconInformation.yHotspot, cursorInformation.hCursor, bitmap.bmWidth, bitmap.bmHeight, 0, NULL, DI_NORMAL)) {
		goto deleteIconBitmap;
	}
deleteIconBitmap:
	DeleteObject(iconInformation.hbmMask);

	if(iconInformation.hbmColor) {
		DeleteObject(iconInformation.hbmColor);
	}
destroyIcon:
	DestroyIcon(icon);
}

BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket, int width, int height, BYTE* screenshotBuffer, BYTE* qoiBuffer, BYTE* previousBuffer) {
	HDC context = GetDC(NULL);
	HDC memoryContext = CreateCompatibleDC(context);
	HBITMAP bitmap = CreateCompatibleBitmap(context, width, height);
	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
	BitBlt(memoryContext, 0, 0, width, height, context, 0, 0, SRCCOPY);
	drawCursor(memoryContext);
	bitmap = SelectObject(memoryContext, oldBitmap);
	BITMAPINFOHEADER header = {sizeof(BITMAPINFOHEADER)};
	header.biWidth = width;
	header.biHeight = height;
	header.biPlanes = 1;
	header.biBitCount = 32;
	BOOL returnValue = FALSE;

	if(!GetDIBits(memoryContext, bitmap, 0, height, screenshotBuffer, (LPBITMAPINFO) &header, DIB_RGB_COLORS)) {
		HackontrolThrowWin32Error(environment, L"GetDIBits");
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
	BYTE run = 0;

	for(int y = height - 1; y >= 0; y--) {
		for(int x = 0; x < width; x++) {
			int bufferIndex = (y * width + x) * 4;
			BYTE red = screenshotBuffer[bufferIndex + 2];
			BYTE green = screenshotBuffer[bufferIndex + 1];
			BYTE blue = screenshotBuffer[bufferIndex];

			if(red == previousRed && green == previousGreen && blue == previousBlue) {
				run++;

				if(run == 62) {
					qoiBuffer[encodedPointer++] = QOI_OP_RUN | (run - 1);
					run = 0;
				}

				continue;
			}

			if(run > 0) {
				qoiBuffer[encodedPointer++] = QOI_OP_RUN | ((run - 1) & 0b111111);
				run = 0;
			}

			BYTE indexPosition = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;

			if(red == seenRed[indexPosition] && green == seenGreen[indexPosition] && blue == seenBlue[indexPosition]) {
				qoiBuffer[encodedPointer++] = QOI_OP_INDEX | indexPosition;
				previousRed = red;
				previousGreen = green;
				previousBlue = blue;
				continue;
			}

			seenRed[indexPosition] = red;
			seenGreen[indexPosition] = green;
			seenBlue[indexPosition] = blue;
			char differenceRed = red - previousRed;
			char differenceGreen = green - previousGreen;
			char differenceBlue = blue - previousBlue;

			if(differenceRed > -3 && differenceRed < 2 && differenceGreen > -3 && differenceGreen < 2 && differenceBlue > -3 && differenceBlue < 2) {
				qoiBuffer[encodedPointer++] = QOI_OP_DIFF | ((differenceRed + 2) << 4) | ((differenceGreen + 2) << 2) | (differenceBlue + 2);
			} else {
				char relativeRed = differenceRed - differenceGreen;
				char relativeBlue = differenceBlue - differenceGreen;

				if(relativeRed > -9 && relativeRed < 8 && differenceGreen > -33 && differenceGreen < 32 && relativeBlue > -9 && relativeBlue < 8) {
					qoiBuffer[encodedPointer++] = QOI_OP_LUMA | (differenceGreen + 32);
					qoiBuffer[encodedPointer++] = ((relativeRed + 8) << 4) | (relativeBlue + 8);
				} else {
					qoiBuffer[encodedPointer++] = QOI_OP_RGB;
					qoiBuffer[encodedPointer++] = red;
					qoiBuffer[encodedPointer++] = green;
					qoiBuffer[encodedPointer++] = blue;
				}
			}

			previousRed = red;
			previousGreen = green;
			previousBlue = blue;
		}
	}

	if(run > 0) {
		qoiBuffer[encodedPointer++] = QOI_OP_RUN | ((run - 1) & 0b111111);
	}

	PACKET packet;
	packet.size = (long) encodedPointer;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = qoiBuffer;

	if(!SendPacket(clientSocket, &packet)) {
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
