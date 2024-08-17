#include "frame_decoder.h"
#include "logger.h"

void DecodeHRSPFrame(const BYTE* data, size_t size, PSTREAMDATA stream, HWND window) {
	if(!data || size < 9 || !stream || !window) {
		return;
	}

	WaitForSingleObject(stream->lock, INFINITE);
	unsigned char flags = data[0];
	BOOL boundaryDifference = flags & 1;
	BOOL colorDifference = (flags >> 1) & 1;

	if(!boundaryDifference || !colorDifference) {
		goto releaseMutex;
	}

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

	InvalidateRect(window, NULL, FALSE);
releaseMutex:
	ReleaseMutex(stream->lock);
}
