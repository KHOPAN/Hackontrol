#include <stdio.h>
#include "definition.h"

BYTE* screenshot(size_t* outputSize) {
	HDC screenContext = GetDC(NULL);
	HDC memoryContext = CreateCompatibleDC(screenContext);
	unsigned int width = GetDeviceCaps(screenContext, HORZRES);
	unsigned int height = GetDeviceCaps(screenContext, VERTRES);
	HBITMAP bitmap = CreateCompatibleBitmap(screenContext, width, height);
	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
	BitBlt(memoryContext, 0, 0, width, height, screenContext, 0, 0, SRCCOPY);
	bitmap = SelectObject(memoryContext, oldBitmap);
	BITMAPINFOHEADER header = {sizeof(BITMAPINFOHEADER)};
	header.biWidth = width;
	header.biHeight = height;
	header.biPlanes = 1;
	header.biBitCount = 32;
	DWORD size = width * height * 4;
	BYTE* data = malloc(size);

	if(data == NULL) {
		return (BYTE*) -1;
	}

	unsigned int result = GetDIBits(memoryContext, bitmap, 0, height, data, (BITMAPINFO*) &header, DIB_RGB_COLORS);
	
	if(result == 0) {
		return (BYTE*) -2;
	}

	BYTE* flippedImage = malloc(size);

	if(flippedImage == NULL) {
		return (BYTE*) -1;
	}

	for(unsigned int y = 0; y < height; y++) {
		for(unsigned int x = 0; x < width; x++) {
			unsigned int position = (y * width + x) * 4;
			unsigned int newPosition = ((height - y - 1) * width + x) * 4;
			flippedImage[position + 0] = data[newPosition + 2];
			flippedImage[position + 1] = data[newPosition + 1];
			flippedImage[position + 2] = data[newPosition + 0];
			flippedImage[position + 3] = data[newPosition + 3];
		}
	}

	free(data);
	BYTE* pngResult = NULL;
	size_t pngSize = 0;
	result = lodepng_encode32(&pngResult, &pngSize, flippedImage, width, height);

	if(result != 0) {
		return (BYTE*) -3;
	}

	free(flippedImage);
	DeleteObject(memoryContext);
	DeleteObject(screenContext);
	*outputSize = pngSize;
	return pngResult;
}
