#include "libkhopan.h"
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

BOOL KHOPANArrayInitialize(_Out_ const PARRAYLIST list, _In_ const size_t size) {
	if(list) {
		for(size_t i = 0; i < sizeof(ARRAYLIST); i++) {
			((PBYTE) list)[i] = 0;
		}
	}

	if(!list || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PBYTE buffer = KHOPAN_ALLOCATE(size * KHOPAN_ARRAY_INITIAL_CAPACITY);

	if(KHOPAN_ALLOCATE_ERROR(buffer)) {
		SetLastError(KHOPAN_ALLOCATE_WIN32_CODE);
		return FALSE;
	}

	list->size = size;
	list->capacity = KHOPAN_ARRAY_INITIAL_CAPACITY;
	list->data = buffer;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayAdd(_Inout_ const PARRAYLIST list, _In_ const PBYTE data) {
	if(!list || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PBYTE buffer;
	size_t index;

	if(list->count >= list->capacity) {
		size_t size = list->size * list->capacity;
		buffer = KHOPAN_ALLOCATE(size * KHOPAN_ARRAY_SCALE_FACTOR);

		if(KHOPAN_ALLOCATE_ERROR(buffer)) {
			SetLastError(KHOPAN_ALLOCATE_WIN32_CODE);
			return FALSE;
		}

		for(index = 0; index < size; index++) {
			buffer[index] = list->data[index];
		}

		KHOPAN_FREE(list->data);
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

BOOL KHOPANArrayRemove(_Inout_ const PARRAYLIST list, _In_ const size_t index) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(list->count > 0 && index >= list->count) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	list->count--;

	if(!list->count) {
		SetLastError(ERROR_SUCCESS);
		return TRUE;
	}

	PBYTE buffer = list->data + list->size * index;

	for(size_t i = 0; i < (list->count - index) * list->size; i++) {
		buffer[i] = buffer[i + list->size];
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayGet(_In_ const PARRAYLIST list, _In_ const size_t index, _Out_ PBYTE* const data) {
	if(data) {
		*data = 0;
	}

	if(!list || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(index >= list->count) {
		SetLastError(ERROR_INDEX_OUT_OF_BOUNDS);
		return FALSE;
	}

	*data = list->data + list->size * index;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANArrayFree(_Inout_ const PARRAYLIST list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	list->count = 0;
	list->size = 0;
	list->capacity = 0;

	if(list->data) {
		KHOPAN_FREE(list->data);
		list->data = NULL;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedInitialize(_Out_ const PLINKEDLIST list, _In_ const size_t size) {
	if(list) {
		for(size_t i = 0; i < sizeof(LINKEDLIST); i++) {
			((PBYTE) list)[i] = 0;
		}
	}

	if(!list || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	list->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!list->mutex) {
		return FALSE;
	}

	list->size = size;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedAdd(_Inout_ const PLINKEDLIST list, _In_ const PBYTE data, _Out_opt_ const PPLINKEDLISTITEM item) {
	if(item) {
		*item = NULL;
	}

	if(!list || !data) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PLINKEDLISTITEM buffer = (PLINKEDLISTITEM) KHOPAN_ALLOCATE(sizeof(LINKEDLISTITEM));

	if(KHOPAN_ALLOCATE_ERROR(buffer)) {
		SetLastError(KHOPAN_ALLOCATE_WIN32_CODE);
		return FALSE;
	}

	buffer->data = KHOPAN_ALLOCATE(list->size);
	size_t index;

	if(KHOPAN_ALLOCATE_ERROR(buffer->data)) {
		index = KHOPAN_ALLOCATE_WIN32_CODE;
		KHOPAN_FREE(buffer);
		SetLastError((DWORD) index);
		return FALSE;
	}

	for(index = 0; index < list->size; index++) {
		buffer->data[index] = data[index];
	}

	buffer->list = list;
	buffer->previous = NULL;
	buffer->next = NULL;

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED) {
		index = KHOPAN_ALLOCATE_WIN32_CODE;
		KHOPAN_FREE(buffer->data);
		KHOPAN_FREE(buffer);
		SetLastError((DWORD) index);
		return FALSE;
	}

	if(list->last) {
		list->last->next = buffer;
		buffer->previous = list->last;
	} else {
		list->first = buffer;
	}

	list->last = buffer;

	if(item) {
		*item = buffer;
	}

	list->count++;
	ReleaseMutex(list->mutex);
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedRemove(const PLINKEDLISTITEM item) {
	/*if(!item) {
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
	SetLastError(ERROR_SUCCESS);*/
	return TRUE;
}

BOOL KHOPANLinkedGet(_In_ const PLINKEDLIST list, _In_ const size_t index, _Out_ const PPLINKEDLISTITEM item) {
	if(item) {
		*item = NULL;
	}

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

	for(PLINKEDLISTITEM listItem = list->first; listItem; listItem = listItem->next) {
		if(index == count) {
			*item = listItem;
			break;
		}

		count++;
	}

	ReleaseMutex(list->mutex);
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedFree(_Inout_ const PLINKEDLIST list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(WaitForSingleObject(list->mutex, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	CloseHandle(list->mutex);
	PLINKEDLISTITEM item = list->last;

	while(item) {
		PLINKEDLISTITEM buffer = item->previous;
		KHOPAN_FREE(item->data);
		KHOPAN_FREE(item);
		item = buffer;
	}

	list->mutex = NULL;
	list->count = 0;
	list->size = 0;
	list->first = NULL;
	list->last = NULL;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
