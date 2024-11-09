#pragma once

#include "libkhopan.h"

#define KHOPAN_ARRAY_INITIAL_CAPACITY 10
#define KHOPAN_ARRAY_SCALE_FACTOR     2

#define KHOPAN_LINKED_LIST_ITERATE(variable, list) for(variable=((PLINKEDLIST)list)->first;variable;variable=((PLINKEDLISTITEM)variable)->next)
#define KHOPAN_LINKED_LIST_ITERATE_REVERSE(variable, list) for(variable=((PLINKEDLIST)list)->last;variable;variable=((PLINKEDLISTITEM)variable)->previous)

typedef struct {
	size_t size;
	size_t capacity;
	PBYTE data;
} DATASTREAM, *PDATASTREAM;

typedef struct {
	size_t count;
	size_t size;
	size_t capacity;
	PBYTE data;
} ARRAYLIST, *PARRAYLIST;

typedef struct _LINKEDLIST LINKEDLIST, *PLINKEDLIST;
typedef struct _LINKEDLISTITEM LINKEDLISTITEM, *PLINKEDLISTITEM, **PPLINKEDLISTITEM;

struct _LINKEDLISTITEM {
	PBYTE data;
	PLINKEDLIST list;
	PLINKEDLISTITEM previous;
	PLINKEDLISTITEM next;
};

struct _LINKEDLIST {
	size_t count;
	size_t size;
	PLINKEDLISTITEM first;
	PLINKEDLISTITEM last;
};

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANStreamInitialize(const PDATASTREAM stream, const size_t size, const PKHOPANERROR error);
BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size, const PKHOPANERROR error);
BOOL KHOPANStreamFree(const PDATASTREAM stream, const PKHOPANERROR error);
BOOL KHOPANArrayInitialize(const PARRAYLIST list, const size_t size, const PKHOPANERROR error);
BOOL KHOPANArrayAdd(const PARRAYLIST list, const PBYTE data, const PKHOPANERROR error);
BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index, const PKHOPANERROR error);
BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, PBYTE* const data, const PKHOPANERROR error);
BOOL KHOPANArrayFree(const PARRAYLIST list, const PKHOPANERROR error);
BOOL KHOPANLinkedInitialize(_Out_ const PLINKEDLIST list, _In_ const size_t size);
BOOL KHOPANLinkedAdd(_Inout_ const PLINKEDLIST list, _In_ const PBYTE data, _Out_opt_ const PPLINKEDLISTITEM item);
BOOL KHOPANLinkedRemove(_In_ const PLINKEDLISTITEM item);
BOOL KHOPANLinkedGet(_In_ const PLINKEDLIST list, _In_ const size_t index, _Out_ const PPLINKEDLISTITEM item);
BOOL KHOPANLinkedFree(_Inout_ const PLINKEDLIST list);
#ifdef __cplusplus
}
#endif
