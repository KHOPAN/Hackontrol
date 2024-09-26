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

	list->count = 0;
	list->size = size;
	list->item = NULL;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedAdd(const PLINKEDLIST list, const PBYTE data, const PPLINKEDLISTITEM item) {
	if(!list || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PLINKEDLISTITEM buffer = LocalAlloc(LMEM_FIXED, sizeof(LINKEDLISTITEM));

	if(!buffer) {
		return FALSE;
	}

	//buffer->data = LocalAlloc(LMEM_FIXED, list->size);
	size_t index;

	/*if(!buffer->data) {
		index = GetLastError();
		LocalFree(buffer);
		SetLastError((DWORD) index);
		return FALSE;
	}

	for(index = 0; index < list->size; index++) {
		buffer->data[index] = data[index];
	}*/

	buffer->list = list;
	buffer->previous = NULL;
	buffer->next = NULL;

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED) {
		index = GetLastError();
		LocalFree(buffer->data);
		LocalFree(buffer);
		SetLastError((DWORD) index);
		return FALSE;
	}

	PLINKEDLISTITEM listItem = list->item;
	PLINKEDLISTITEM previous = NULL;

	while(listItem) {
		previous = listItem;
		listItem = listItem->next;
	}

	if(previous) {
		buffer->previous = previous;
		previous->next = buffer;
	} else {
		list->item = buffer;
	}

	list->count++;
	ReleaseMutex(list->mutex);

	if(item) {
		(*item) = buffer;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedRemove(const PLINKEDLISTITEM item) {
	if(!item) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PLINKEDLIST list = item->list;

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	item->previous->next = item->next;
	item->next->previous = item->previous;
	LocalFree(item->data);
	LocalFree(item);
	list->count--;
	ReleaseMutex(list->mutex);
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedGet(const PLINKEDLIST list, const size_t index, const PPLINKEDLISTITEM item) {
	if(!list || !item) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index >= list->count) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	size_t count = 0;

	for(PLINKEDLISTITEM listItem = list->item; listItem; listItem = listItem->next) {
		if(index == count) {
			(*item) = listItem;
			break;
		}

		count++;
	}

	ReleaseMutex(list->mutex);
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedFree(const PLINKEDLIST list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	CloseHandle(list->mutex);
	PLINKEDLISTITEM item = list->item;

	while(item) {
		PLINKEDLISTITEM buffer = item->next;
		//LocalFree(item->data);
		LocalFree(item);
		item = buffer;
	}

	list->count = 0;
	list->size = 0;
	list->mutex = NULL;
	list->item = NULL;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
