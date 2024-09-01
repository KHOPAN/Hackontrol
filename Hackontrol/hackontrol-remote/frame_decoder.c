#include "frame_decoder.h"

#define SUBTRACT do{if(colorDifference){window->stream.pixels[pixelIndex]-=blue;window->stream.pixels[pixelIndex+1]-=green;window->stream.pixels[pixelIndex+2]-=red;}else{window->stream.pixels[pixelIndex]=blue;window->stream.pixels[pixelIndex+1]=green;window->stream.pixels[pixelIndex+2]=red;}}while(0)

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000
#define OP_MASK      0b11000000

void DecodeHRSPFrame(const BYTE* data, const size_t size, const PWINDOWDATA window) {
	if(!data || size < 9 || !window || !window->stream.streaming || WaitForSingleObject(window->lock, INFINITE) == WAIT_FAILED) {
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

		PostMessageW(window->window, WM_SIZE, 0, 0);
	}

	if(!window->stream.pixels) {
		goto releaseMutex;
	}

	int x;
	int y;
	int pixelIndex;
	int dataIndex;

	if(!boundaryDifference || !colorDifference) {
		goto exitRawPixel;
	}

	if(size - 9 < width * height * 3) {
		goto releaseMutex;
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			pixelIndex = (y * width + x) * 4;
			dataIndex = ((height - y - 1) * width + x) * 3;
			window->stream.pixels[pixelIndex] = data[dataIndex + 11];
			window->stream.pixels[pixelIndex + 1] = data[dataIndex + 10];
			window->stream.pixels[pixelIndex + 2] = data[dataIndex + 9];
		}
	}

	goto invalidateWindow;
exitRawPixel:
	if(boundaryDifference && size - 9 < 16) {
		goto releaseMutex;
	}

	int startX = boundaryDifference ? (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12] : 0;
	int endX = boundaryDifference ? (data[17] << 24) | (data[18] << 16) | (data[19] << 8) | data[20] : width - 1;
	int endY = boundaryDifference ? (data[21] << 24) | (data[22] << 16) | (data[23] << 8) | data[24] : height - 1;
	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	memset(seenRed, 0, sizeof(seenRed));
	memset(seenGreen, 0, sizeof(seenGreen));
	memset(seenBlue, 0, sizeof(seenBlue));
	size_t pointer = boundaryDifference ? 25 : 9;
	int red = 0;
	int green = 0;
	int blue = 0;
	int run = 0;
	int temporary = 0;

	for(y = boundaryDifference ? (data[13] << 24) | (data[14] << 16) | (data[15] << 8) | data[16] : 0; y <= endY; y++) {
		for(x = startX; x <= endX; x++) {
			pixelIndex = ((height - y - 1) * width + x) * 4;

			if(run > 0) {
				SUBTRACT;
				run--;
				continue;
			}

			if(size - pointer < 1) {
				goto releaseMutex;
			}

			temporary = data[pointer++];

			if(temporary == QOI_OP_RGB) {
				if(size - pointer < 3) goto releaseMutex;
				red = data[pointer++];
				green = data[pointer++];
				blue = data[pointer++];
				dataIndex = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
				seenRed[dataIndex] = red;
				seenGreen[dataIndex] = green;
				seenBlue[dataIndex] = blue;
				SUBTRACT;
				continue;
			}

			switch(temporary & OP_MASK) {
			case QOI_OP_INDEX: {
				dataIndex = temporary & 0b111111;
				red = seenRed[dataIndex];
				green = seenGreen[dataIndex];
				blue = seenBlue[dataIndex];
				break;
			}
			case QOI_OP_DIFF:
				red += ((temporary >> 4) & 0b11) - 2;
				green += ((temporary >> 2) & 0b11) - 2;
				blue += (temporary & 0b11) - 2;
				break;
			case QOI_OP_LUMA:
				if(size - pointer < 1) goto releaseMutex;
				dataIndex = data[pointer++];
				temporary = (temporary & 0b111111) - 32;
				red += temporary - 8 + ((dataIndex >> 4) & 0b1111);
				green += temporary;
				blue += temporary - 8 + (dataIndex & 0b1111);
				break;
			case QOI_OP_RUN:
				SUBTRACT;
				run = (temporary & 0b111111);
				continue;
			}

			dataIndex = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
			seenRed[dataIndex] = red;
			seenGreen[dataIndex] = green;
			seenBlue[dataIndex] = blue;
			SUBTRACT;
		}
	}
invalidateWindow:
	InvalidateRect(window->window, NULL, FALSE);
releaseMutex:
	ReleaseMutex(window->lock);
}
