#include "libkhopan.h"
#include "libkhopanlist.h"

#pragma warning(disable: 6386)

BOOL KHOPANStreamInitialize(_Out_ const PDATASTREAM stream, _In_opt_ const size_t size) {
	if(stream) {
		for(size_t i = 0; i < sizeof(DATASTREAM); i++) {
			((PBYTE) stream)[i] = 0;
		}
	}

	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(size) {
		stream->data = KHOPAN_ALLOCATE(size);

		if(KHOPAN_ALLOCATE_ERROR(stream->data)) {
			SetLastError(KHOPAN_ALLOCATE_WIN32_CODE);
			return FALSE;
		}
	} else {
		stream->data = NULL;
	}

	stream->capacity = size;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANStreamAdd(_Inout_ const PDATASTREAM stream, _In_ const PBYTE data, _In_ const size_t size) {
	if(!stream || !data || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	size_t length = stream->size + size;
	size_t index;

	if(length > stream->capacity) {
		PBYTE buffer = KHOPAN_ALLOCATE(length);

		if(KHOPAN_ALLOCATE_ERROR(buffer)) {
			SetLastError(KHOPAN_ALLOCATE_WIN32_CODE);
			return FALSE;
		}

		if(stream->data) {
			for(index = 0; index < stream->size; index++) buffer[index] = stream->data[index];
			KHOPAN_FREE(stream->data);
		}

		stream->capacity = length;
		stream->data = buffer;
	}

	for(index = 0; index < size; index++) {
		stream->data[stream->size + index] = data[index];
	}

	stream->size += size;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANStreamFree(_Inout_ const PDATASTREAM stream) {
	if(!stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	stream->size = 0;
	stream->capacity = 0;

	if(stream->data) {
		KHOPAN_FREE(stream->data);
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
	if(!list || !list->count) {
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

	if(!list || !data || !list->count) {
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

	list->size = size;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedAdd(_Inout_ const PLINKEDLIST list, _In_ const PBYTE data, _Out_opt_ const PPLINKEDLISTITEM item) {
	if(item) {
		*item = NULL;
	}

	if(!list || !data || !list->size) {
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
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

BOOL KHOPANLinkedRemove(_In_ const PLINKEDLISTITEM item) {
	if(!item) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PLINKEDLIST list = item->list;

	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(item->previous) {
		item->previous->next = item->next;
	}

	if(item->next) {
		item->next->previous = item->previous;
	}

	if(item->data) {
		KHOPAN_FREE(item->data);
	}

	KHOPAN_FREE(item);
	list->count--;
	SetLastError(ERROR_SUCCESS);
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

	size_t count = 0;
	BOOL found = FALSE;
	PLINKEDLISTITEM listItem;

	KHOPAN_LINKED_LIST_ITERATE(listItem, list) {
		if(index == count) {
			*item = listItem;
			found = TRUE;
			break;
		}

		count++;
	}

	SetLastError(found ? ERROR_SUCCESS : ERROR_INDEX_OUT_OF_BOUNDS);
	return TRUE;
}

BOOL KHOPANLinkedFree(_Inout_ const PLINKEDLIST list) {
	if(!list) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	PLINKEDLISTITEM item = list->last;

	while(item) {
		PLINKEDLISTITEM buffer = item->previous;

		if(item->data) {
			KHOPAN_FREE(item->data);
		}

		KHOPAN_FREE(item);
		item = buffer;
	}

	list->count = 0;
	list->size = 0;
	list->first = NULL;
	list->last = NULL;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
