#include "frame_decoder.h"
#include "logger.h"

#define SUBTRACT do{if(colorDifference){window->stream.pixels[pixelIndex]-=blue;window->stream.pixels[pixelIndex+1]-=green;window->stream.pixels[pixelIndex+2]-=red;}else{window->stream.pixels[pixelIndex]=blue;window->stream.pixels[pixelIndex+1]=green;window->stream.pixels[pixelIndex+2]=red;}}while(0)

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000
#define OP_MASK      0b11000000

void DecodeHRSPFrame(const BYTE* data, const size_t size, const PWINDOWDATA window) {
	if(!data || size < 9 || !window || WaitForSingleObject(window->lock, INFINITE) == WAIT_FAILED) {
		return;
	}

	BOOL boundaryDifference = data[0] & 1;
	BOOL colorDifference = (data[0] >> 1) & 1;
	int width = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
	int height = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];

	if(window->stream.originalImageWidth != width || window->stream.originalImageHeight != height) {
		window->stream.originalImageWidth = width;
		window->stream.originalImageHeight = height;

		if(window->stream.pixels) {
			LocalFree(window->stream.pixels);
		}

		window->stream.pixels = LocalAlloc(LMEM_FIXED, width * height * 4);

		if(!window->stream.pixels) {
			goto releaseMutex;
		}

		RECT bounds;
		GetClientRect(window->window, &bounds);
		PostMessageW(window->window, WM_SIZE, 0, MAKELONG(bounds.right - bounds.left, bounds.bottom - bounds.top));
	}

	if(!window->stream.pixels) {
		goto releaseMutex;
	}

	if(boundaryDifference && colorDifference) {
		if(size - 9 < width * height * 3) {
			goto releaseMutex;
		}

		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				int pixelIndex = (y * width + x) * 4;
				int dataIndex = ((height - y - 1) * width + x) * 3;
				window->stream.pixels[pixelIndex] = data[dataIndex + 11];
				window->stream.pixels[pixelIndex + 1] = data[dataIndex + 10];
				window->stream.pixels[pixelIndex + 2] = data[dataIndex + 9];
			}
		}

		goto invalidateWindow;
	}

	if(boundaryDifference && size - 9 < 16) {
		goto releaseMutex;
	}

	int startX = boundaryDifference ? (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12] : 0;
	int startY = boundaryDifference ? (data[13] << 24) | (data[14] << 16) | (data[15] << 8) | data[16] : 0;
	int endX = boundaryDifference ? (data[17] << 24) | (data[18] << 16) | (data[19] << 8) | data[20] : width - 1;
	int endY = boundaryDifference ? (data[21] << 24) | (data[22] << 16) | (data[23] << 8) | data[24] : height - 1;
	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	memset(seenRed, 0, sizeof(seenRed));
	memset(seenGreen, 0, sizeof(seenGreen));
	memset(seenBlue, 0, sizeof(seenBlue));
	int red = 0;
	int green = 0;
	int blue = 0;
	int run = 0;
	size_t pointer = boundaryDifference ? 25 : 9;

	for(int y = startY; y <= endY; y++) {
		for(int x = startX; x <= endX; x++) {
			int pixelIndex = ((height - y - 1) * width + x) * 4;

			if(run > 0) {
				SUBTRACT;
				run--;
				continue;
			}

			if(size - pointer < 1) {
				goto releaseMutex;
			}

			BYTE chunk = data[pointer++];
			int index;

			if(chunk == QOI_OP_RGB) {
				if(size - pointer < 3) goto releaseMutex;
				red = data[pointer++];
				green = data[pointer++];
				blue = data[pointer++];
				index = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
				seenRed[index] = red;
				seenGreen[index] = green;
				seenBlue[index] = blue;
				SUBTRACT;
				continue;
			}

			switch(chunk & OP_MASK) {
			case QOI_OP_INDEX: {
				index = chunk & 0b111111;
				red = seenRed[index];
				green = seenGreen[index];
				blue = seenBlue[index];
				break;
			}
			case QOI_OP_DIFF:
				red += ((chunk >> 4) & 0b11) - 2;
				green += ((chunk >> 2) & 0b11) - 2;
				blue += (chunk & 0b11) - 2;
				break;
			case QOI_OP_LUMA:
				if(size - pointer < 1) goto releaseMutex;
				index = data[pointer++];
				int differenceGreen = (chunk & 0b111111) - 32;
				red += differenceGreen - 8 + ((index >> 4) & 0b1111);
				green += differenceGreen;
				blue += differenceGreen - 8 + (index & 0b1111);
				break;
			case QOI_OP_RUN:
				SUBTRACT;
				run = (chunk & 0b111111);
				continue;
			}

			index = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
			seenRed[index] = red;
			seenGreen[index] = green;
			seenBlue[index] = blue;
			SUBTRACT;
		}
	}
invalidateWindow:
	InvalidateRect(window->window, NULL, FALSE);
releaseMutex:
	ReleaseMutex(window->lock);
}
