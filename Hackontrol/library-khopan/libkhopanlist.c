#include "libkhopanlist.h"

//#define ERROR_WIN32(sourceName, functionName)             if(error){error->facility=ERROR_FACILITY_WIN32;error->code=GetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)
//#define ERROR_SOURCE(sourceName)                          if(error){error->source=sourceName;}

#pragma warning(disable: 6386)

BOOL KHOPANStreamInitialize(const PDATASTREAM stream, const size_t size, const PKHOPANERROR error) {
	if(!stream) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamInitialize", NULL);
		return FALSE;
	}

	if(size) {
		stream->data = KHOPAN_ALLOCATE(size);

		if(!stream->data) {
			ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamInitialize", L"KHOPAN_ALLOCATE");
			return FALSE;
		}
	} else {
		stream->data = NULL;
	}

	stream->size = 0;
	stream->capacity = size;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size, const PKHOPANERROR error) {
	if(!stream || !data || !size) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamAdd", NULL);
		return FALSE;
	}

	size_t length = stream->size + size;
	size_t index;

	if(length > stream->capacity) {
		PBYTE buffer = KHOPAN_ALLOCATE(length);

		if(!buffer) {
			ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamAdd", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		if(stream->data) {
			for(index = 0; index < stream->size; index++) buffer[index] = stream->data[index];
			KHOPAN_DEALLOCATE(stream->data);
		}

		stream->capacity = length;
		stream->data = buffer;
	}

	for(index = 0; index < size; index++) {
		stream->data[stream->size + index] = data[index];
	}

	stream->size += size;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANStreamFree(const PDATASTREAM stream, const PKHOPANERROR error) {
	if(!stream) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANStreamFree", NULL);
		return FALSE;
	}

	stream->size = 0;
	stream->capacity = 0;

	if(stream->data) {
		KHOPAN_DEALLOCATE(stream->data);
		stream->data = NULL;
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayInitialize(const PARRAYLIST list, const size_t size, const PKHOPANERROR error) {
	if(!list || !size) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayInitialize", NULL);
		return FALSE;
	}

	PBYTE buffer = KHOPAN_ALLOCATE(size * KHOPAN_ARRAY_INITIAL_CAPACITY);

	if(KHOPAN_ALLOCATE_FAILED(buffer)) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayInitialize", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	list->count = 0;
	list->size = size;
	list->capacity = KHOPAN_ARRAY_INITIAL_CAPACITY;
	list->data = buffer;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayAdd(const PARRAYLIST list, const PBYTE data, const PKHOPANERROR error) {
	if(!list || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayAdd", NULL);
		return FALSE;
	}

	PBYTE buffer;
	size_t index;

	if(list->count >= list->capacity) {
		size_t size = list->size * list->capacity;
		buffer = KHOPAN_ALLOCATE(size * KHOPAN_ARRAY_SCALE_FACTOR);

		if(!buffer) {
			ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayAdd", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		for(index = 0; index < size; index++) {
			buffer[index] = list->data[index];
		}

		KHOPAN_DEALLOCATE(list->data);
		list->capacity *= KHOPAN_ARRAY_SCALE_FACTOR;
		list->data = buffer;
	}

	buffer = list->data + list->size * list->count;

	for(index = 0; index < list->size; index++) {
		buffer[index] = data[index];
	}

	list->count++;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index, const PKHOPANERROR error) {
	if(!list) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayRemove", NULL);
		return FALSE;
	}

	if(index >= list->count) {
		ERROR_COMMON(ERROR_COMMON_INDEX_OUT_OF_BOUNDS, L"KHOPANArrayRemove", NULL);
		return FALSE;
	}

	list->count--;

	if(!list->count) {
		ERROR_CLEAR;
		return TRUE;
	}

	PBYTE buffer = list->data + list->size * index;

	for(size_t i = 0; i < (list->count - index) * list->size; i++) {
		buffer[i] = buffer[i + list->size];
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, PBYTE* const data, const PKHOPANERROR error) {
	if(!list || !data || !list->count) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayGet", NULL);
		return FALSE;
	}

	if(index >= list->count) {
		ERROR_COMMON(ERROR_COMMON_INDEX_OUT_OF_BOUNDS, L"KHOPANArrayGet", NULL);
		return FALSE;
	}

	*data = list->data + list->size * index;
	ERROR_CLEAR;
	return TRUE;
}

BOOL KHOPANArrayFree(const PARRAYLIST list, const PKHOPANERROR error) {
	if(!list) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"KHOPANArrayFree", NULL);
		return FALSE;
	}

	list->count = 0;
	list->size = 0;
	list->capacity = 0;

	if(list->data) {
		KHOPAN_DEALLOCATE(list->data);
		list->data = NULL;
	}

	ERROR_CLEAR;
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

	PLINKEDLISTITEM buffer = KHOPAN_ALLOCATE(sizeof(LINKEDLISTITEM));

	if(KHOPAN_ALLOCATE_FAILED(buffer)) {
		SetLastError(KHOPAN_ALLOCATE_ERROR);
		return FALSE;
	}

	buffer->data = KHOPAN_ALLOCATE(list->size);
	size_t index;

	if(KHOPAN_ALLOCATE_FAILED(buffer->data)) {
		index = KHOPAN_ALLOCATE_ERROR;
		KHOPAN_DEALLOCATE(buffer);
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

	if(list->first == item) {
		list->first = item->next;
	}

	if(list->last == item) {
		list->last = item->previous;
	}

	if(item->previous) {
		item->previous->next = item->next;
	}

	if(item->next) {
		item->next->previous = item->previous;
	}

	if(item->data) {
		KHOPAN_DEALLOCATE(item->data);
	}

	KHOPAN_DEALLOCATE(item);
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
			KHOPAN_DEALLOCATE(item->data);
		}

		KHOPAN_DEALLOCATE(item);
		item = buffer;
	}

	list->count = 0;
	list->size = 0;
	list->first = NULL;
	list->last = NULL;
	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
