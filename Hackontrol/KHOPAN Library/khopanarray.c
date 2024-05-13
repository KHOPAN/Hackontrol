#include <stdio.h>
#include "khopanarray.h"

BOOL KHArrayInitialize(DynamicArray* dynamicArray, size_t elementSize) {
	if(!dynamicArray) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, elementSize * DYNAMIC_ARRAY_INITIAL_CAPACITY);

	if(!buffer) {
		return FALSE;
	}

	dynamicArray->data = buffer;
	dynamicArray->elementSize = elementSize;
	dynamicArray->elementCount = 0;
	dynamicArray->capacity = DYNAMIC_ARRAY_INITIAL_CAPACITY;
	return TRUE;
}

BOOL KHArraySize(DynamicArray* dynamicArray, size_t* arraySize) {
	if(!dynamicArray || !arraySize) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	(*arraySize) = dynamicArray->elementCount;
	return TRUE;
}
BOOL KHArrayIsEmpty(DynamicArray* dynamicArray, LPBOOL isEmpty) {
	if(!dynamicArray || !isEmpty) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	(*isEmpty) = dynamicArray->elementCount == 0;
	return TRUE;
}

BOOL KHArrayAdd(DynamicArray* dynamicArray, void* data) {
	if(!dynamicArray) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(dynamicArray->elementCount >= dynamicArray->capacity) {
		size_t dataSizeInBytes = dynamicArray->elementSize * dynamicArray->capacity;
		size_t newSize = dataSizeInBytes * DYNAMIC_ARRAY_SCALE_FACTOR;
		BYTE* buffer = LocalAlloc(LMEM_FIXED, newSize);

		if(!buffer) {
			return FALSE;
		}

		memcpy(buffer, dynamicArray->data, dataSizeInBytes);
		LocalFree(dynamicArray->data);
		dynamicArray->data = buffer;
		dynamicArray->capacity *= DYNAMIC_ARRAY_SCALE_FACTOR;
	}

	memcpy((void*) (((size_t) (*dynamicArray).data) + dynamicArray->elementCount * dynamicArray->elementSize), data, dynamicArray->elementSize);
	dynamicArray->elementCount++;
	return TRUE;
}

BOOL KHArrayGet(DynamicArray* dynamicArray, size_t index, void** data) {
	if(!dynamicArray || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index < 0 || index >= dynamicArray->elementCount) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	(*data) = (void*) (((size_t) (*dynamicArray).data) + index * dynamicArray->elementSize);
	return TRUE;
}

BOOL KHArrayFree(DynamicArray* dynamicArray) {
	if(!dynamicArray) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(dynamicArray->data && LocalFree(dynamicArray->data)) {
		return FALSE;
	}

	return TRUE;
}
