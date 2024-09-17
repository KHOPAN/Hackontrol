#include "khopanlist.h"

BOOL KHOPANArrayInitialize(const PARRAYLIST list, size_t size) {
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
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, const PBYTE* data) {
	if(!data) {
		goto success;
	}

	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
success:
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

BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size) {
	if(!data || !size) {
		goto success;
	}

	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
success:
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANStreamFree(const PDATASTREAM stream) {
	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
