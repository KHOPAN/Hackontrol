#include "frame_decoder.h"

BOOL DecodeHRSPFrame(const BYTE* data, size_t size, int width, int height, HBITMAP* output) {
	if(!data || !output) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HBITMAP bitmap = CreateBitmap(width, height, 1, 32, NULL);

	if(!bitmap) {
		return FALSE;
	}

	(*output) = bitmap;
	return TRUE;
}
