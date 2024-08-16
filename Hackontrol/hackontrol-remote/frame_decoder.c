#include "frame_decoder.h"
#include "logger.h"

void DecodeHRSPFrame(const BYTE* data, size_t size, PSTREAMDATA stream) {
	if(!data || size < 9 || !stream) {
		return;
	}

	unsigned char flags = data[0];
	BOOL boundaryDifference = flags & 1;
	BOOL colorDifference = (flags >> 1) & 1;

	if(!boundaryDifference || !colorDifference) {
		return;
	}

	int width = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
	int height = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];
	LOG("Frame: %dx%d\n" COMMA width COMMA height);
}
