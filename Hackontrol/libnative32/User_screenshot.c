#include <khopanstring.h>
#include <khopanjava.h>
#include "exception.h"
#include "User.h"
#include <lodepng.h>

static BOOL drawCursor(JNIEnv* const environment, const HDC context) {
	CURSORINFO cursorInformation;
	cursorInformation.cbSize = sizeof(CURSORINFO);

	if(!GetCursorInfo(&cursorInformation)) {
		HackontrolThrowWin32Error(environment, L"GetCursorInfo");
		return FALSE;
	}

	if(cursorInformation.flags != CURSOR_SHOWING) {
		return TRUE;
	}

	HICON icon = CopyIcon(cursorInformation.hCursor);

	if(!icon) {
		HackontrolThrowWin32Error(environment, L"CopyIcon");
		return FALSE;
	}

	ICONINFO iconInformation;
	BOOL returnValue = FALSE;

	if(!GetIconInfo(icon, &iconInformation)) {
		HackontrolThrowWin32Error(environment, L"GetIconInfo");
		goto destroyIcon;
	}

	/*if(!DrawIcon(context, 20, 20, icon)) {
		HackontrolThrowWin32Error(environment, L"DrawIcon");
		goto deleteIconBitmap;
	}*/

	BITMAP bitmap;

	if(!GetObjectW(iconInformation.hbmMask, sizeof(bitmap), &bitmap)) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"GetObjectW");
		goto deleteIconBitmap;
	}

	/*if(!DrawIconEx(context, 20, 20, icon, bitmap.bmWidth, bitmap.bmHeight, 0, NULL, DI_NORMAL)) {
		HackontrolThrowWin32Error(environment, L"DrawIconEx");
		goto deleteIconBitmap;
	}*/

	HDC cursorContext = CreateCompatibleDC(context);
	HBITMAP old = SelectObject(cursorContext, iconInformation.hbmMask);
	//BitBlt(context, 20, 20, bitmap.bmWidth, bitmap.bmHeight, cursorContext, 0, 0, SRCCOPY);
	MaskBlt(context, 200, 200, bitmap.bmWidth, bitmap.bmHeight, cursorContext, 0, 0, iconInformation.hbmMask, 0, 0, MAKEROP4(SRCPAINT, SRCCOPY));
	SelectObject(cursorContext, old);
	DeleteDC(cursorContext);
	//MessageBoxW(NULL, KHFormatMessageW(L"Color: %ws\nMask: %ws", iconInformation.hbmColor ? L"Present" : L"null", iconInformation.hbmMask ? L"Present" : L"null"), L"Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL | MB_DEFBUTTON1);
	returnValue = TRUE;
deleteIconBitmap:
	DeleteObject(iconInformation.hbmMask);
	DeleteObject(iconInformation.hbmColor);
destroyIcon:
	DestroyIcon(icon);
	return returnValue;
}

jbyteArray User_screenshot(JNIEnv* const environment, const jclass class) {
	HDC context = GetDC(NULL);
	int width = GetDeviceCaps(context, HORZRES);
	int height = GetDeviceCaps(context, VERTRES);
	HDC memoryContext = CreateCompatibleDC(context);
	HBITMAP bitmap = CreateCompatibleBitmap(context, width, height);
	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
	BitBlt(memoryContext, 0, 0, width, height, context, 0, 0, SRCCOPY);
	jbyteArray returnValue = NULL;

	if(!drawCursor(environment, memoryContext)) {
		goto cleanup;
	}

	bitmap = SelectObject(memoryContext, oldBitmap);
	DWORD size = 4 * width * height;
	BYTE* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto cleanup;
	}

	BITMAPINFOHEADER header = { sizeof(BITMAPINFOHEADER) };
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

	BYTE* flippedImage = LocalAlloc(LMEM_FIXED, size);

	if(!flippedImage) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		LocalFree(buffer);
		goto cleanup;
	}

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int position = (y * width + x) * 4;
			int newPosition = ((height - y - 1) * width + x) * 4;
			flippedImage[position + 0] = buffer[newPosition + 2];
			flippedImage[position + 1] = buffer[newPosition + 1];
			flippedImage[position + 2] = buffer[newPosition + 0];
			flippedImage[position + 3] = buffer[newPosition + 3];
		}
	}

	LocalFree(buffer);
	BYTE* pngImage;
	size_t pngSize;
	unsigned int result = lodepng_encode32(&pngImage, &pngSize, flippedImage, width, height);

	if(result) {
		LPWSTR message = KHFormatMessageW(L"lodepng_encode32() %S (Error code: %u)", lodepng_error_text(result), result);
		
		if(!message) {
			goto freeFlippedImage;
		}

		KHJavaThrowInternalErrorW(environment, message);
		LocalFree(message);
		goto freeFlippedImage;
	}

	jsize byteArraySize = (jsize) pngSize;
	jbyteArray array = (*environment)->NewByteArray(environment, byteArraySize);

	if(!array) {
		free(pngImage);
		goto freeFlippedImage;
	}

	(*environment)->SetByteArrayRegion(environment, array, 0, byteArraySize, pngImage);
	returnValue = array;
	free(pngImage);
freeFlippedImage:
	LocalFree(flippedImage);
cleanup:
	DeleteObject(bitmap);
	DeleteDC(memoryContext);
	ReleaseDC(NULL, context);
	return returnValue;
}
