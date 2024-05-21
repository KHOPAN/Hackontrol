#pragma once

#include <Windows.h>

typedef struct {
	void* data;
	size_t size;
} DataStream;

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHDataStreamAdd(DataStream* stream, void* data, size_t size);
BOOL KHDataStreamFree(DataStream* stream);
#ifdef __cplusplus
}
#endif
