#pragma once

#include <Windows.h>

#define KHOPAN_ARRAY_INITIAL_CAPACITY 10
#define KHOPAN_ARRAY_SCALE_FACTOR     2

typedef struct {
	size_t size;
	PBYTE data;
} DATASTREAM, *PDATASTREAM;

typedef struct {
	size_t count;
	size_t size;
	size_t capacity;
	PBYTE data;
} ARRAYLIST, *PARRAYLIST;

struct _LINKEDLIST;

typedef struct _LINKEDLISTITEM {
	PBYTE data;
	struct _LINKEDLIST* list;
	struct _LINKEDLISTITEM* previous;
	struct _LINKEDLISTITEM* next;
} LINKEDLISTITEM, *PLINKEDLISTITEM, **PPLINKEDLISTITEM;

typedef struct _LINKEDLIST {
	size_t count;
	size_t size;
	HANDLE mutex;
	PLINKEDLISTITEM item;
} LINKEDLIST, *PLINKEDLIST;

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size);
BOOL KHOPANStreamFree(const PDATASTREAM stream);
BOOL KHOPANArrayInitialize(_Out_ const PARRAYLIST list, _In_ const size_t size);
BOOL KHOPANArrayAdd(_Inout_ const PARRAYLIST list, _In_ const PBYTE data);
BOOL KHOPANArrayRemove(_Inout_ const PARRAYLIST list, _In_ const size_t index);
BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, PBYTE* const data);
BOOL KHOPANArrayFree(_Inout_ const PARRAYLIST list);
BOOL KHOPANLinkedInitialize(const PLINKEDLIST list, const size_t size);
BOOL KHOPANLinkedAdd(const PLINKEDLIST list, const PBYTE data, const PPLINKEDLISTITEM item);
BOOL KHOPANLinkedRemove(const PLINKEDLISTITEM item);
BOOL KHOPANLinkedGet(const PLINKEDLIST list, const size_t index, const PPLINKEDLISTITEM item);
BOOL KHOPANLinkedFree(const PLINKEDLIST list);
#ifdef __cplusplus
}
#endif
