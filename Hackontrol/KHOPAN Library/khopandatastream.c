#include "khopandatastream.h"

static BOOL allocateDataStreamBuffer(DataStream* stream, void* data, size_t size) {
	void* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return FALSE;
	}

	memcpy(buffer, data, size);
	stream->data = buffer;
	stream->size = size;
	return TRUE;
}

BOOL KHDataStreamAdd(DataStream* stream, void* data, size_t size) {
	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(!stream->data || !stream->size) {
		return allocateDataStreamBuffer(stream, data, size);
	}

	size_t bufferSize = size + stream->size;
	BYTE* buffer = LocalAlloc(LMEM_FIXED, bufferSize);

	if(!buffer) {
		return FALSE;
	}

	memcpy(buffer, stream->data, stream->size);
	memcpy(buffer + stream->size, data, size);

	if(LocalFree(stream->data)) {
		return FALSE;
	}

	stream->data = buffer;
	stream->size = bufferSize;
	return TRUE;
}

BOOL KHDataStreamFree(DataStream* stream) {
	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(stream->data && LocalFree(stream->data)) {
		stream->data = NULL;
		return FALSE;
	}

	return TRUE;
}
