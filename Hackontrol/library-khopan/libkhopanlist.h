#pragma once

#include "libkhopan.h"

#define KHOPAN_ARRAY_INITIAL_CAPACITY 10
#define KHOPAN_ARRAY_SCALE_FACTOR     2

#define KHOPAN_LINKED_LIST_ITERATE_FORWARD(variable, list)  for((variable)=((PLINKEDLIST)(list))->first;(variable);(variable)=((PLINKEDLISTITEM)(variable))->next)
#define KHOPAN_LINKED_LIST_ITERATE_BACKWARD(variable, list) for((variable)=((PLINKEDLIST)(list))->last;(variable);(variable)=((PLINKEDLISTITEM)(variable))->previous)

typedef struct {
	size_t size;
	size_t capacity;
	LPVOID data;
} DATASTREAM, *PDATASTREAM;

typedef struct {
	size_t count;
	size_t size;
	size_t capacity;
	LPVOID data;
} ARRAYLIST, *PARRAYLIST;

typedef struct _LINKEDLIST LINKEDLIST, *PLINKEDLIST;
typedef struct _LINKEDLISTITEM LINKEDLISTITEM, *PLINKEDLISTITEM, **PPLINKEDLISTITEM;

struct _LINKEDLIST {
	size_t count;
	size_t size;
	PLINKEDLISTITEM first;
	PLINKEDLISTITEM last;
};

struct _LINKEDLISTITEM {
	LPVOID data;
	PLINKEDLIST list;
	PLINKEDLISTITEM previous;
	PLINKEDLISTITEM next;
};

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANStreamInitialize(const PDATASTREAM stream, const size_t size, const PKHOPANERROR error);
BOOL KHOPANStreamAdd(const PDATASTREAM stream, const LPVOID data, const size_t size, const PKHOPANERROR error);
BOOL KHOPANStreamFree(const PDATASTREAM stream, const PKHOPANERROR error);
BOOL KHOPANArrayInitialize(const PARRAYLIST list, const size_t size, const PKHOPANERROR error);
BOOL KHOPANArrayAdd(const PARRAYLIST list, const LPVOID data, const PKHOPANERROR error);
BOOL KHOPANArrayRemove(const PARRAYLIST list, const size_t index, const PKHOPANERROR error);
BOOL KHOPANArrayGet(const PARRAYLIST list, const size_t index, LPVOID* const data, const PKHOPANERROR error);
BOOL KHOPANArrayFree(const PARRAYLIST list, const PKHOPANERROR error);
BOOL KHOPANLinkedInitialize(const PLINKEDLIST list, const size_t size, const PKHOPANERROR error);
BOOL KHOPANLinkedAdd(const PLINKEDLIST list, const LPVOID data, const PPLINKEDLISTITEM item, const PKHOPANERROR error);
BOOL KHOPANLinkedRemove(const PLINKEDLISTITEM item, const PKHOPANERROR error);
BOOL KHOPANLinkedGet(const PLINKEDLIST list, const size_t index, const PPLINKEDLISTITEM item, const PKHOPANERROR error);
BOOL KHOPANLinkedFree(const PLINKEDLIST list, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANDownloadData(const PDATASTREAM stream, const LPCSTR address, const BOOL initialized, const BOOL force, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif
