#include "frame_decoder.h"

void DecodeHRSPFrame(const BYTE* data, size_t size, PSTREAMDATA stream) {
	if(!data || size < 1 || !stream) {
		return;
	}

	unsigned char flags = data[0];
	BOOL boundaryDifference = flags & 1;
	BOOL colorDifference = (flags >> 1) & 1;

	if(!boundaryDifference || !colorDifference) {
		return;
	}
}
