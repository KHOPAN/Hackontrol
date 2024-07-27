#pragma once

#include <Windows.h>

#define ARRAY_LIST_INITIAL_CAPACITY 10
#define ARRAY_LIST_SCALE_FACTOR     2

typedef struct {
	void* data;
	size_t elementSize;
	size_t elementCount;
	size_t capacity;
} ArrayList;

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHArrayInitialize(ArrayList* list, size_t elementSize);
BOOL KHArraySize(ArrayList* list, size_t* arraySize);
BOOL KHArrayIsEmpty(ArrayList* list, LPBOOL isEmpty);
BOOL KHArrayAdd(ArrayList* list, void* data);
BOOL KHArrayRemove(ArrayList* list, size_t index);
BOOL KHArrayGet(ArrayList* list, size_t index, void** data);
BOOL KHArrayFree(ArrayList* list);
#ifdef __cplusplus
}
#endif
