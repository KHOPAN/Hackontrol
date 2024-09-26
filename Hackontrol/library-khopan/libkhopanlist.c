#include "libkhopanlist.h"

BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size) {
	if(!stream || !data || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	size_t length = stream->data ? stream->size : 0;
	PBYTE buffer = LocalAlloc(LMEM_FIXED, length + size);

	if(!buffer) {
		return FALSE;
	}

	size_t index;

	if(stream->data) {
		for(index = 0; index < length; index++) {
			buffer[index] = stream->data[index];
		}

		LocalFree(stream->data);
	}

	for(index = 0; index < size; index++) {
		buffer[index + length] = data[index];
	}

	stream->size = length + size;
	stream->data = buffer;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANStreamFree(const PDATASTREAM stream) {
	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	stream->size = 0;

	if(stream->data) {
		LocalFree(stream->data);
		stream->data = NULL;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayInitialize(const PARRAYLIST list, const size_t size) {
	if(!list || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, size * KHOPAN_ARRAY_INITIAL_CAPACITY);

	if(!buffer) {
		return FALSE;
	}

	list->count = 0;
	list->size = size;
	list->capacity = KHOPAN_ARRAY_INITIAL_CAPACITY;
	list->data = buffer;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayAdd(const PARRAYLIST list, const PBYTE data) {
	if(!list || !data || !list->size || !list->capacity || !list->data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PBYTE buffer;
	size_t index;

	if(list->count >= list->capacity) {
		size_t size = list->size * list->capacity;
		buffer = LocalAlloc(LMEM_FIXED, size * KHOPAN_ARRAY_SCALE_FACTOR);

		if(!buffer) {
			return FALSE;
		}

		for(index = 0; index < size; index++) {
			buffer[index] = list->data[index];
		}

		LocalFree(list->data);
		list->capacity *= KHOPAN_ARRAY_SCALE_FACTOR;
		list->data = buffer;
	}

	buffer = list->data + list->size * list->capacity;

	for(index = 0; index < list->size; index++) {
		buffer[index] = data[index];
	}

	list->count++;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index) {
	if(!list || !list->size || !list->data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index >= list->count) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	list->count--;

	if(!list->count) {
		SetLastError(ERROR_SUCCESS);
		return TRUE;
	}

	PBYTE target = list->data + list->size * index;

	for(size_t i = 0; i < (list->count - index) * list->size; i++) {
		target[i] = target[i + list->size];
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, PBYTE* const data) {
	if(!list || !data || !list->size || !list->data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index >= list->count) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	(*data) = list->data + list->size * index;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayFree(const PARRAYLIST list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(list->data) {
		LocalFree(list->data);
		list->count = 0;
		list->size = 0;
		list->capacity = 0;
		list->data = NULL;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedInitialize(const PLINKEDLIST list, const size_t size) {
	if(!list || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	list->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!list->mutex) {
		return FALSE;
	}

	list->size = size;
	list->item = NULL;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedAdd(const PLINKEDLIST list, const PPLINKEDLISTITEM item) {
	if(!list || !item) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedFree(const PLINKEDLIST list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED || !ReleaseMutex(list->mutex)) {
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
