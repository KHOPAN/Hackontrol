#pragma once

#include <Windows.h>

typedef struct {
	size_t count;
	size_t size;
	size_t capacity;
	PBYTE data;
} ARRAYLIST, *PARRAYLIST;

typedef struct {
	size_t size;
	PBYTE data;
} DATASTREAM, *PDATASTREAM;

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANArrayInitialize(const PARRAYLIST list, size_t size);
BOOL KHOPANArrayAdd(const PARRAYLIST list, const PBYTE data);
BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index);
BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, const PBYTE* data);
BOOL KHOPANArrayFree(const PARRAYLIST list);
BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size);
BOOL KHOPANStreamFree(const PDATASTREAM stream);
#ifdef __cplusplus
}
#endif
