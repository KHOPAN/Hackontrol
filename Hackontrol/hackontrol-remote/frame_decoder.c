#include "frame_decoder.h"

BOOL DecodeHRSPFrame(const BYTE* data, size_t size, STREAMDATA* stream) {
	if(!data || !stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(size < 1) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}
	
	unsigned char flags = data[0];
	BOOL boundaryDifference = flags & 1;
	BOOL colorDifference = (flags >> 1) & 1;

	if(!boundaryDifference || !colorDifference) {
		return TRUE;
	}

	return TRUE;
}
