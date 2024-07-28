#include "packet.h"
#include <khopanstring.h>
#include <khopanjava.h>
#include <khopandatastream.h>
#include "stream.h"
#include "exception.h"

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000

/*
 *    /------- Send mouse input separately
 *    |
 *    | /----- Send frame with color differences from previous frame
 *    | |
 * 76543210 -- Enable stream frame sending
 *     | |
 *     | \---- Send frame with only the boundary differences
 *     |
 *     \------ Correction frame, send as raw image then set this bit to 0
 * 
 * Note: If bit 1 and 2 were set, uncompresed frame will be send instead (for some reason, I can't get boundary and color difference to work together
 */
static unsigned char globalStreamSettings = 0;

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

static BOOL takeScreenshot(JNIEnv* const environment, PACKET* packet, int width, int height, BYTE* screenshotBuffer, BYTE* qoiBuffer, BYTE* previousBuffer) {
	if(!(globalStreamSettings & 1)) {
		return TRUE;
	}

	BOOL boundaryDifference = (globalStreamSettings >> 1) & 1;
	BOOL colorDifference = (globalStreamSettings >> 2) & 1;

	if(globalStreamSettings & 0b1000) {
		boundaryDifference = FALSE;
		colorDifference = FALSE;
		globalStreamSettings &= 0b11110111;
	}

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
	qoiBuffer[encodedPointer++] = ((colorDifference & 1) << 1) | (boundaryDifference & 1);

	if(boundaryDifference && colorDifference) {
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				int baseIndex = (height - y - 1) * width + x;
				int screenshotIndex = baseIndex * 4;
				int previousIndex = baseIndex * 3;
				qoiBuffer[encodedPointer++] = previousBuffer[previousIndex] = screenshotBuffer[screenshotIndex + 2];
				qoiBuffer[encodedPointer++] = previousBuffer[previousIndex + 1] = screenshotBuffer[screenshotIndex + 1];
				qoiBuffer[encodedPointer++] = previousBuffer[previousIndex + 2] = screenshotBuffer[screenshotIndex];
			}
		}

		goto sendFrame;
	}

	int startX = boundaryDifference ? width - 1 : 0;
	int startY = boundaryDifference ? height - 1 : 0;
	int endX = boundaryDifference ? 0 : width - 1;
	int endY = boundaryDifference ? 0 : height - 1;

	if(boundaryDifference) {
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				int baseIndex = (height - y - 1) * width + x;
				int screenshotIndex = baseIndex * 4;
				int previousIndex = baseIndex * 3;
				if(screenshotBuffer[screenshotIndex + 2] == previousBuffer[previousIndex] && screenshotBuffer[screenshotIndex + 1] == previousBuffer[previousIndex + 1] && screenshotBuffer[screenshotIndex] == previousBuffer[previousIndex + 2]) continue;
				previousBuffer[previousIndex] = screenshotBuffer[screenshotIndex + 2];
				previousBuffer[previousIndex + 1] = screenshotBuffer[screenshotIndex + 1];
				previousBuffer[previousIndex + 2] = screenshotBuffer[screenshotIndex];
				startX = min(startX, x);
				startY = min(startY, y);
				endX = max(endX, x);
				endY = max(endY, y);
			}
		}

		qoiBuffer[encodedPointer++] = (startX >> 24) & 0xFF;
		qoiBuffer[encodedPointer++] = (startX >> 16) & 0xFF;
		qoiBuffer[encodedPointer++] = (startX >> 8) & 0xFF;
		qoiBuffer[encodedPointer++] = startX & 0xFF;
		qoiBuffer[encodedPointer++] = (startY >> 24) & 0xFF;
		qoiBuffer[encodedPointer++] = (startY >> 16) & 0xFF;
		qoiBuffer[encodedPointer++] = (startY >> 8) & 0xFF;
		qoiBuffer[encodedPointer++] = startY & 0xFF;
		qoiBuffer[encodedPointer++] = (endX >> 24) & 0xFF;
		qoiBuffer[encodedPointer++] = (endX >> 16) & 0xFF;
		qoiBuffer[encodedPointer++] = (endX >> 8) & 0xFF;
		qoiBuffer[encodedPointer++] = endX & 0xFF;
		qoiBuffer[encodedPointer++] = (endY >> 24) & 0xFF;
		qoiBuffer[encodedPointer++] = (endY >> 16) & 0xFF;
		qoiBuffer[encodedPointer++] = (endY >> 8) & 0xFF;
		qoiBuffer[encodedPointer++] = endY & 0xFF;
	}

	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	memset(seenRed, 0, sizeof(seenRed));
	memset(seenGreen, 0, sizeof(seenGreen));
	memset(seenBlue, 0, sizeof(seenBlue));
	BYTE run = 0;
	BYTE previousRed = 0;
	BYTE previousGreen = 0;
	BYTE previousBlue = 0;

	for(int y = startY; y <= endY; y++) {
		for(int x = startX; x <= endX; x++) {
			int baseIndex = (height - y - 1) * width + x;
			int screenshotIndex = baseIndex * 4;
			int previousIndex = baseIndex * 3;
			BYTE red = screenshotBuffer[screenshotIndex + 2];
			BYTE green = screenshotBuffer[screenshotIndex + 1];
			BYTE blue = screenshotBuffer[screenshotIndex];

			if(colorDifference) {
				red = previousBuffer[previousIndex] - screenshotBuffer[screenshotIndex + 2];
				green = previousBuffer[previousIndex + 1] - screenshotBuffer[screenshotIndex + 1];
				blue = previousBuffer[previousIndex + 2] - screenshotBuffer[screenshotIndex];
				previousBuffer[previousIndex] = screenshotBuffer[screenshotIndex + 2];
				previousBuffer[previousIndex + 1] = screenshotBuffer[screenshotIndex + 1];
				previousBuffer[previousIndex + 2] = screenshotBuffer[screenshotIndex];
			}

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
sendFrame:
	packet->size = (long) encodedPointer;
	packet->data = qoiBuffer;
	returnValue = TRUE;
cleanup:
	DeleteObject(bitmap);
	DeleteDC(memoryContext);
	ReleaseDC(NULL, context);
	return returnValue;
}

DWORD WINAPI ScreenStreamThread(_In_ STREAMPARAMETER* parameter) {
	if(!parameter) {
		return 1;
	}

	JavaVM* virtualMachine = parameter->virtualMachine;
	JNIEnv* environment = NULL;
	JavaVMAttachArgs arguments = {0};
	arguments.version = JNI_VERSION_21;

	if((*virtualMachine)->AttachCurrentThread(virtualMachine, (void**) &environment, &arguments) != JNI_OK) {
		return 1;
	}

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	size_t baseSize = width * height;
	size_t bufferSize = baseSize * 4;
	BYTE* screenshotBuffer = LocalAlloc(LMEM_FIXED, bufferSize);

	if(!screenshotBuffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto detachThread;
	}

	BYTE* qoiBuffer = LocalAlloc(LMEM_FIXED, bufferSize);

	if(!qoiBuffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto freeScreenshotBuffer;
	}

	BYTE* previousBuffer = LocalAlloc(LMEM_FIXED, baseSize * 3);

	if(!previousBuffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto freeQOIBuffer;
	}

	PACKET packet;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;

	while(TRUE) {
		if(!takeScreenshot(environment, &packet, width, height, screenshotBuffer, qoiBuffer, previousBuffer)) {
			goto freePreviousBuffer;
		}

		if(!SendPacket(parameter->clientSocket, &packet)) {
			HackontrolThrowWin32Error(environment, L"SendPacket");
			goto freePreviousBuffer;
		}
	}
freePreviousBuffer:
	LocalFree(previousBuffer);
freeQOIBuffer:
	LocalFree(qoiBuffer);
freeScreenshotBuffer:
	LocalFree(screenshotBuffer);
detachThread:
	if((*virtualMachine)->DetachCurrentThread(virtualMachine) != JNI_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"JavaVM::DetachCurrentThread");
		return 1;
	}

	return 0;
}

void SetStreamParameter(unsigned char streamSettings) {
	globalStreamSettings = streamSettings;
}
