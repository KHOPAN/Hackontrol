#pragma once

#include <Windows.h>

#define DYNAMIC_ARRAY_INITIAL_CAPACITY 10
#define DYNAMIC_ARRAY_SCALE_FACTOR     2

typedef struct {
	void* data;
	size_t elementSize;
	size_t elementCount;
	size_t capacity;
} DynamicArray;

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHArrayInitialize(DynamicArray* dynamicArray, size_t elementSize);
BOOL KHArraySize(DynamicArray* dynamicArray, size_t* arraySize);
BOOL KHArrayIsEmpty(DynamicArray* dynamicArray, LPBOOL isEmpty);
BOOL KHArrayAdd(DynamicArray* dynamicArray, void* data);
BOOL KHArrayGet(DynamicArray* dynamicArray, size_t index, void** data);
BOOL KHArrayFree(DynamicArray* dynamicArray);
#ifdef __cplusplus
}
#endif
