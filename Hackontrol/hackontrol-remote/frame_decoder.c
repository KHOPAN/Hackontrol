#include "frame_decoder.h"
#include "logger.h"

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000
#define OP_MASK      0b11000000

void DecodeHRSPFrame(const BYTE* data, size_t size, PSTREAMDATA stream, HWND window) {
	if(!data || size < 9 || !stream || !window) {
		return;
	}

	WaitForSingleObject(stream->lock, INFINITE);
	unsigned char flags = data[0];
	BOOL boundaryDifference = flags & 1;
	BOOL colorDifference = (flags >> 1) & 1;
	int width = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
	int height = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];

	if(stream->width != width || stream->height != height) {
		stream->width = width;
		stream->height = height;

		if(stream->pixels) {
			LocalFree(stream->pixels);
		}

		stream->pixels = LocalAlloc(LMEM_FIXED, width * height * 4);

		if(!stream->pixels) {
			goto releaseMutex;
		}
	}

	if(boundaryDifference && colorDifference) {
		if(size - 9 < width * height * 3) {
			goto releaseMutex;
		}

		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				int pixelIndex = (y * width + x) * 4;
				int dataIndex = ((height - y - 1) * width + x) * 3;
				stream->pixels[pixelIndex] = data[dataIndex + 11];
				stream->pixels[pixelIndex + 1] = data[dataIndex + 10];
				stream->pixels[pixelIndex + 2] = data[dataIndex + 9];
			}
		}

		goto invalidateWindow;
	}

	if(boundaryDifference && size - 9 < 16) {
		goto releaseMutex;
	}

	size_t pointer = 9;
	int startX = boundaryDifference ? (data[pointer++] << 24) | ((data[pointer++] & 0xFF) << 16) | ((data[pointer++] & 0xFF) << 8) | (data[pointer++] & 0xFF) : 0;
	int startY = boundaryDifference ? (data[pointer++] << 24) | ((data[pointer++] & 0xFF) << 16) | ((data[pointer++] & 0xFF) << 8) | (data[pointer++] & 0xFF) : 0;
	int endX = boundaryDifference ? (data[pointer++] << 24) | ((data[pointer++] & 0xFF) << 16) | ((data[pointer++] & 0xFF) << 8) | (data[pointer++] & 0xFF) : width - 1;
	int endY = boundaryDifference ? (data[pointer++] << 24) | ((data[pointer++] & 0xFF) << 16) | ((data[pointer++] & 0xFF) << 8) | (data[pointer++] & 0xFF) : height - 1;
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
#define SUBTRACT(x,y,z) do{if(colorDifference){stream->pixels[pixelIndex]-=x;stream->pixels[pixelIndex+1]-=y;stream->pixels[pixelIndex+2]-=z;}else{stream->pixels[pixelIndex]=x;stream->pixels[pixelIndex+1]=y;stream->pixels[pixelIndex+2]=z;}}while(0)

	for(int y = startY; y <= endY; y++) {
		for(int x = startX; x <= endX; x++) {
			int pixelIndex = (y * width + x) * 4;

			if(run > 0) {
				SUBTRACT(red, green, blue);
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
				SUBTRACT(red, green, blue);
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
				SUBTRACT(red, green, blue);
				run = (chunk & 0b111111);
				continue;
			}

			index = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
			seenRed[index] = red;
			seenGreen[index] = green;
			seenBlue[index] = blue;
			SUBTRACT(red, green, blue);
		}
	}
invalidateWindow:
	InvalidateRect(window, NULL, FALSE);
releaseMutex:
	ReleaseMutex(stream->lock);
}
