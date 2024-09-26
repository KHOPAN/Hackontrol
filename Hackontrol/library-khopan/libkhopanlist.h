#pragma once

#include <Windows.h>

#define KHOPAN_ARRAY_INITIAL_CAPACITY 10
#define KHOPAN_ARRAY_SCALE_FACTOR     2

typedef struct {
	size_t size;
	PBYTE data;
} DATASTREAM, * PDATASTREAM;

typedef struct {
	size_t count;
	size_t size;
	size_t capacity;
	PBYTE data;
} ARRAYLIST, *PARRAYLIST;

typedef struct _LINKEDLISTITEM {
	PBYTE data;
	struct _LINKEDLISTITEM* previous;
	struct _LINKEDLISTITEM* next;
} LINKEDLISTITEM, *PLINKEDLISTITEM, **PPLINKEDLISTITEM;

typedef struct {
	size_t size;
	HANDLE mutex;
	PLINKEDLISTITEM item;
} LINKEDLIST, *PLINKEDLIST;

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANStreamAdd(const PDATASTREAM stream, const PBYTE data, const size_t size);
BOOL KHOPANStreamFree(const PDATASTREAM stream);
BOOL KHOPANArrayInitialize(const PARRAYLIST list, const size_t size);
BOOL KHOPANArrayAdd(const PARRAYLIST list, const PBYTE data);
BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index);
BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, PBYTE* const data);
BOOL KHOPANArrayFree(const PARRAYLIST list);
BOOL KHOPANLinkedInitialize(const PLINKEDLIST list, const size_t size);
BOOL KHOPANLinkedAdd(const PLINKEDLIST list, const PBYTE data, const PPLINKEDLISTITEM item);
BOOL KHOPANLinkedRemove(const PLINKEDLISTITEM item);
BOOL KHOPANLinkedFree(const PLINKEDLIST list);
#ifdef __cplusplus
}
#endif
