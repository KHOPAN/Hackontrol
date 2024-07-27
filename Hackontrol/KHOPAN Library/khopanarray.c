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

BOOL KHArrayRemove(ArrayList* list, size_t index) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index < 0 || index >= list->elementCount) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	list->elementCount--;

	if(!list->elementCount) {
		return TRUE;
	}

	return TRUE;
}

BOOL KHArrayGet(ArrayList* list, size_t index, void** data) {
	if(!list || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index < 0 || index >= list->elementCount) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	(*data) = (void*) (((size_t) (*list).data) + index * list->elementSize);
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
