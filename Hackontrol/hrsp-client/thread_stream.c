#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

#define ERROR_WIN32(errorCode, functionName) if(!parameter->hasError){parameter->hasError=TRUE;parameter->error.type=HRSP_CLIENT_ERROR_TYPE_WIN32;parameter->error.code=errorCode;parameter->error.function=functionName;}

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000

#pragma warning(disable: 6386)
#pragma warning(disable: 6387)

static void rawEncode(const UINT width, const UINT height, const PBYTE buffer, const PBYTE previousBuffer, const PULONG pointer) {
	for(UINT y = 0; y < height; y++) {
		for(UINT x = 0; x < width; x++) {
			ULONG baseIndex = (height - y - 1) * width + x;
			ULONG screenshotIndex = baseIndex * 4;
			ULONG previousIndex = baseIndex * 3;
			buffer[(*pointer)++] = previousBuffer[previousIndex] = buffer[screenshotIndex + 2];
			buffer[(*pointer)++] = previousBuffer[previousIndex + 1] = buffer[screenshotIndex + 1];
			buffer[(*pointer)++] = previousBuffer[previousIndex + 2] = buffer[screenshotIndex];
		}
	}
}

static void findBoundary(const PUINT startX, const PUINT startY, const PUINT endX, const PUINT endY, const UINT width, const UINT height, const PBYTE buffer, const PBYTE previousBuffer) {
	for(UINT y = 0; y < height; y++) {
		for(UINT x = 0; x < width; x++) {
			ULONG baseIndex = (height - y - 1) * width + x;
			ULONG screenshotIndex = baseIndex * 4;
			ULONG previousIndex = baseIndex * 3;
			if(buffer[screenshotIndex + 2] == previousBuffer[previousIndex] && buffer[screenshotIndex + 1] == previousBuffer[previousIndex + 1] && buffer[screenshotIndex] == previousBuffer[previousIndex + 2]) continue;
			previousBuffer[previousIndex] = buffer[screenshotIndex + 2];
			previousBuffer[previousIndex + 1] = buffer[screenshotIndex + 1];
			previousBuffer[previousIndex + 2] = buffer[screenshotIndex];
			(*startX) = min((*startX), x);
			(*startY) = min((*startY), y);
			(*endX) = max((*endX), x);
			(*endY) = max((*endY), y);
		}
	}
}

static void qoiEncode(const UINT startX, const UINT startY, const UINT endX, const UINT endY, const UINT width, const UINT height, const PBYTE buffer, const PBYTE previousBuffer, const PULONG pointer, const BOOL colorDifference) {
	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	UINT x;

	for(x = 0; x < 64; x++) {
		seenRed[x] = 0;
		seenGreen[x] = 0;
		seenBlue[x] = 0;
	}

	BYTE previousRed = 0;
	BYTE previousGreen = 0;
	BYTE previousBlue = 0;
	BYTE runLength = 0;
	BYTE indexPosition;

	for(UINT y = startY; y <= endY; y++) {
		for(x = startX; x <= endX; x++) {
			ULONG baseIndex = (height - y - 1) * width + x;
			ULONG screenshotIndex = baseIndex * 4;
			ULONG previousIndex = baseIndex * 3;
			BYTE red = buffer[screenshotIndex + 2];
			BYTE green = buffer[screenshotIndex + 1];
			BYTE blue = buffer[screenshotIndex];
			if(!colorDifference) goto colorNormalized;
			red = previousBuffer[previousIndex] - buffer[screenshotIndex + 2];
			green = previousBuffer[previousIndex + 1] - buffer[screenshotIndex + 1];
			blue = previousBuffer[previousIndex + 2] - buffer[screenshotIndex];
			previousBuffer[previousIndex] = buffer[screenshotIndex + 2];
			previousBuffer[previousIndex + 1] = buffer[screenshotIndex + 1];
			previousBuffer[previousIndex + 2] = buffer[screenshotIndex];
		colorNormalized:
			if(previousRed != red || previousGreen != green || previousBlue != blue) goto differentPixel;
			runLength++;
			if(runLength != 62) continue;
			buffer[(*pointer)++] = QOI_OP_RUN | (runLength - 1);
			runLength = 0;
			continue;
		differentPixel:
			if(runLength < 1) goto zeroRunLength;
			buffer[(*pointer)++] = QOI_OP_RUN | ((runLength - 1) & 0b111111);
			runLength = 0;
		zeroRunLength:
			indexPosition = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
			if(seenRed[indexPosition] != red || seenGreen[indexPosition] != green || seenBlue[indexPosition] != blue) goto notInIndexTable;
			buffer[(*pointer)++] = QOI_OP_INDEX | indexPosition;
			goto setPrevious;
		notInIndexTable:
			seenRed[indexPosition] = red;
			seenGreen[indexPosition] = green;
			seenBlue[indexPosition] = blue;
			char differenceRed = red - previousRed;
			char differenceGreen = green - previousGreen;
			char differenceBlue = blue - previousBlue;
			if(differenceRed > -3 && differenceRed < 2 && differenceGreen > -3 && differenceGreen < 2 && differenceBlue > -3 && differenceBlue < 2) goto smallDifference;
			differenceRed -= differenceGreen;
			differenceBlue -= differenceGreen;
			if(differenceRed > -9 && differenceRed < 8 && differenceGreen > -33 && differenceGreen < 32 && differenceBlue > -9 && differenceBlue < 8) goto largeDifference;
			buffer[(*pointer)++] = QOI_OP_RGB;
			buffer[(*pointer)++] = red;
			buffer[(*pointer)++] = green;
			buffer[(*pointer)++] = blue;
			goto setPrevious;
		largeDifference:
			buffer[(*pointer)++] = QOI_OP_LUMA | (differenceGreen + 32);
			buffer[(*pointer)++] = ((differenceRed + 8) << 4) | (differenceBlue + 8);
			goto setPrevious;
		smallDifference:
			buffer[(*pointer)++] = QOI_OP_DIFF | ((differenceRed + 2) << 4) | ((differenceGreen + 2) << 2) | (differenceBlue + 2);
		setPrevious:
			previousRed = red;
			previousGreen = green;
			previousBlue = blue;
		}
	}

	if(runLength > 0) {
		buffer[(*pointer)++] = QOI_OP_RUN | ((runLength - 1) & 0b111111);
	}
}

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	UINT oldWidth = 0;
	UINT oldHeight = 0;
	ULONG offsetEncoded = 0;
	ULONG offsetPrevious = 0;
	PBYTE buffer = NULL;
	HDC context = NULL;
	HDC memoryContext = NULL;
	HBITMAP bitmap = NULL;

	while(parameter->running) {
		if(WaitForSingleObject(parameter->mutex, INFINITE) == WAIT_FAILED) {
			ERROR_WIN32(GetLastError(), L"WaitForSingleObject");
			break;
		}

		if(!(parameter->stream.flags & 1)) {
			ReleaseMutex(parameter->mutex);
			continue;
		}

		BYTE boundaryDifference = (parameter->stream.flags >> 1) & 1;
		BYTE colorDifference = (parameter->stream.flags >> 2) & 1;

		if(parameter->stream.flags & 0b1000) {
			boundaryDifference = TRUE;
			colorDifference = TRUE;
			parameter->stream.flags &= 0b11110111;
		}

		ReleaseMutex(parameter->mutex);
		UINT width = GetSystemMetrics(SM_CXSCREEN);
		UINT height = GetSystemMetrics(SM_CYSCREEN);

		if(oldWidth == width && oldHeight == height && buffer && context && memoryContext && bitmap) {
			goto captureFrame;
		}

		oldWidth = width;
		oldHeight = height;
		offsetEncoded = width * height * 4;
		offsetPrevious = offsetEncoded * 2;

		if(buffer) {
			LocalFree(buffer);
		}

		buffer = LocalAlloc(LMEM_FIXED, width * height * 11);

		if(!buffer) {
			ERROR_WIN32(GetLastError(), L"LocalAlloc");
			break;
		}

		if(context) {
			ReleaseDC(NULL, context);
		}

		context = GetDC(NULL);

		if(!context) {
			ERROR_WIN32(ERROR_FUNCTION_FAILED, L"GetDC");
			break;
		}

		if(memoryContext) {
			DeleteDC(memoryContext);
		}

		memoryContext = CreateCompatibleDC(context);

		if(!memoryContext) {
			ERROR_WIN32(ERROR_FUNCTION_FAILED, L"CreateCompatibleDC");
			break;
		}

		if(bitmap) {
			DeleteObject(bitmap);
		}

		bitmap = CreateCompatibleBitmap(context, width, height);

		if(!bitmap) {
			ERROR_WIN32(ERROR_FUNCTION_FAILED, L"CreateCompatibleBitmap");
			break;
		}
	captureFrame:
		bitmap = SelectObject(memoryContext, bitmap);

		if(!BitBlt(memoryContext, 0, 0, width, height, context, 0, 0, SRCCOPY)) {
			ERROR_WIN32(GetLastError(), L"BitBlt");
			bitmap = SelectObject(memoryContext, bitmap);
			break;
		}

		CURSORINFO cursorInformation;
		cursorInformation.cbSize = sizeof(CURSORINFO);

		if(!GetCursorInfo(&cursorInformation) || cursorInformation.flags != CURSOR_SHOWING) {
			goto finishDrawCursor;
		}

		HICON icon = CopyIcon(cursorInformation.hCursor);

		if(!icon) {
			goto finishDrawCursor;
		}

		ICONINFO iconInformation;

		if(!GetIconInfo(icon, &iconInformation)) {
			DestroyIcon(icon);
			goto finishDrawCursor;
		}

		if(iconInformation.hbmColor) {
			BITMAP iconBitmap;
			if(GetObjectW(iconInformation.hbmColor, sizeof(iconBitmap), &iconBitmap)) DrawIconEx(memoryContext, cursorInformation.ptScreenPos.x - iconInformation.xHotspot, cursorInformation.ptScreenPos.y - iconInformation.yHotspot, cursorInformation.hCursor, iconBitmap.bmWidth, iconBitmap.bmHeight, 0, NULL, DI_NORMAL);
		}

		DeleteObject(iconInformation.hbmMask);

		if(iconInformation.hbmColor) {
			DeleteObject(iconInformation.hbmColor);
		}

		DestroyIcon(icon);
	finishDrawCursor:
		bitmap = SelectObject(memoryContext, bitmap);
		BITMAPINFO information = {0};
		information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		information.bmiHeader.biWidth = width;
		information.bmiHeader.biHeight = height;
		information.bmiHeader.biPlanes = 1;
		information.bmiHeader.biBitCount = 32;
		information.bmiHeader.biCompression = BI_RGB;

		if(!GetDIBits(memoryContext, bitmap, 0, height, buffer, &information, DIB_RGB_COLORS)) {
			ERROR_WIN32(ERROR_FUNCTION_FAILED, L"GetDIBits");
			break;
		}

		ULONG pointer = offsetEncoded;
		buffer[pointer++] = ((colorDifference & 1) << 1) | (boundaryDifference & 1);
		buffer[pointer++] = (width >> 24) & 0xFF;
		buffer[pointer++] = (width >> 16) & 0xFF;
		buffer[pointer++] = (width >> 8) & 0xFF;
		buffer[pointer++] = width & 0xFF;
		buffer[pointer++] = (height >> 24) & 0xFF;
		buffer[pointer++] = (height >> 16) & 0xFF;
		buffer[pointer++] = (height >> 8) & 0xFF;
		buffer[pointer++] = height & 0xFF;
		PBYTE previousBuffer = buffer + offsetPrevious;
		HRSPPACKET packet;

		if(colorDifference && boundaryDifference) {
			rawEncode(width, height, buffer, previousBuffer, &pointer);
			goto framePacket;
		}

		UINT startX = boundaryDifference ? width - 1 : 0;
		UINT startY = boundaryDifference ? height - 1 : 0;
		UINT endX = boundaryDifference ? 0 : width - 1;
		UINT endY = boundaryDifference ? 0 : height - 1;

		if(boundaryDifference) {
			findBoundary(&startX, &startY, &endX, &endY, width, height, buffer, previousBuffer);
			buffer[pointer++] = (startX >> 24) & 0xFF;
			buffer[pointer++] = (startX >> 16) & 0xFF;
			buffer[pointer++] = (startX >> 8) & 0xFF;
			buffer[pointer++] = startX & 0xFF;
			buffer[pointer++] = (startY >> 24) & 0xFF;
			buffer[pointer++] = (startY >> 16) & 0xFF;
			buffer[pointer++] = (startY >> 8) & 0xFF;
			buffer[pointer++] = startY & 0xFF;
			buffer[pointer++] = (endX >> 24) & 0xFF;
			buffer[pointer++] = (endX >> 16) & 0xFF;
			buffer[pointer++] = (endX >> 8) & 0xFF;
			buffer[pointer++] = endX & 0xFF;
			buffer[pointer++] = (endY >> 24) & 0xFF;
			buffer[pointer++] = (endY >> 16) & 0xFF;
			buffer[pointer++] = (endY >> 8) & 0xFF;
			buffer[pointer++] = endY & 0xFF;
		}

		qoiEncode(startX, startY, endX, endY, width, height, buffer, previousBuffer, &pointer, colorDifference);
	framePacket:
		packet.size = (int) (pointer - offsetEncoded);
		packet.type = HRSP_REMOTE_CLIENT_STREAM_FRAME_PACKET;
		packet.data = buffer + offsetEncoded;
		HRSPERROR protocolError;

		if(!HRSPSendPacket(parameter->socket, &parameter->data, &packet, &protocolError)) {
			parameter->hasError = TRUE;
			parameter->error.type = protocolError.win32 ? HRSP_CLIENT_ERROR_TYPE_WIN32 : HRSP_CLIENT_ERROR_TYPE_HRSP;
			parameter->error.function = protocolError.function;
			parameter->error.code = protocolError.code;
			break;
		}
	}

	if(context) {
		ReleaseDC(NULL, context);
	}

	if(memoryContext) {
		DeleteDC(memoryContext);
	}

	if(bitmap) {
		DeleteObject(bitmap);
	}

	if(buffer) {
		LocalFree(buffer);
	}

	if(parameter->hasError) {
		closesocket(parameter->socket);
		return 1;
	}

	return 0;
}
