#include <stdio.h>
#include "khopanarray.h"

BOOL KHArrayInitialize(ArrayList* list, size_t elementSize) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, elementSize * ARRAY_LIST_INITIAL_CAPACITY);

	if(!buffer) {
		return FALSE;
	}

	list->data = buffer;
	list->elementSize = elementSize;
	list->elementCount = 0;
	list->capacity = ARRAY_LIST_INITIAL_CAPACITY;
	return TRUE;
}

BOOL KHArraySize(ArrayList* list, size_t* arraySize) {
	if(!list || !arraySize) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	(*arraySize) = list->elementCount;
	return TRUE;
}

BOOL KHArrayIsEmpty(ArrayList* list, LPBOOL isEmpty) {
	if(!list || !isEmpty) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	(*isEmpty) = list->elementCount == 0;
	return TRUE;
}

BOOL KHArrayAdd(ArrayList* list, void* data) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(list->elementCount >= list->capacity) {
		size_t dataSizeInBytes = list->elementSize * list->capacity;
		size_t newSize = dataSizeInBytes * ARRAY_LIST_SCALE_FACTOR;
		BYTE* buffer = LocalAlloc(LMEM_FIXED, newSize);

		if(!buffer) {
			return FALSE;
		}

		memcpy(buffer, list->data, dataSizeInBytes);
		LocalFree(list->data);
		list->data = buffer;
		list->capacity *= ARRAY_LIST_SCALE_FACTOR;
	}

	memcpy((void*) (((size_t) (*list).data) + list->elementCount * list->elementSize), data, list->elementSize);
	list->elementCount++;
	return TRUE;
}

static inline BYTE* getDataPointerAtIndex(ArrayList* list, size_t index) {
	return (BYTE*) list->data + list->elementSize * index;
}

BOOL KHArrayRemove(ArrayList* list, size_t index) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index >= list->elementCount) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	list->elementCount--;

	if(!list->elementCount) {
		return TRUE;
	}

	BYTE* removeTarget = getDataPointerAtIndex(list, index);

	for(size_t i = 0; i < (list->elementCount - index) * list->elementSize; i++) {
		removeTarget[i] = removeTarget[i + list->elementSize];
	}

	return TRUE;
}

BOOL KHArrayGet(ArrayList* list, size_t index, void** data) {
	if(!list || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index >= list->elementCount) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	(*data) = getDataPointerAtIndex(list, index);
	return TRUE;
}

BOOL KHArrayFree(ArrayList* list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(list->data && LocalFree(list->data)) {
		return FALSE;
	}

	return TRUE;
}
