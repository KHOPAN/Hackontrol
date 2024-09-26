#include <libkhopan.h>
#include <libkhopanlist.h>

int main(int argc, char** argv) {
	/*LINKEDLIST list;

	if(!KHOPANLinkedInitialize(&list, sizeof(ULONGLONG))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedInitialize");
		return 1;
	}

	Sleep(5000);
	int codeExit = 1;

	for(ULONGLONG i = 1; i <= 10000; i++) {
		if(!KHOPANLinkedAdd(&list, (PBYTE) &i, NULL)) {
			KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedAdd");
			goto freeList;
		}
	}

	/*PLINKEDLISTITEM pointer = NULL;

	if(!KHOPANLinkedGet(&list, 3659, &pointer)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedGet");
		goto freeList;
	}

	if(!KHOPANLinkedRemove(pointer)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedRemove");
		goto freeList;
	}*/

	/*for(PLINKEDLISTITEM item = list.item; item; item = item->next) {
		printf("Number: %llu\n", *((PULONGLONG) item->data));
	}*/

	/*Sleep(2000);
	codeExit = 0;
freeList:
	if(!KHOPANLinkedFree(&list)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedFree");
		codeExit = 1;
	}

	return codeExit;*/
	ARRAYLIST list = {0};

	if(!KHOPANArrayInitialize(&list, sizeof(ULONGLONG))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANArrayInitialize");
		return 1;
	}

	Sleep(2000);
	int codeExit = 1;

	for(ULONGLONG i = 0; i <= 10000; i++) {
		if(!KHOPANArrayAdd(&list, (PBYTE) &i)) {
			KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANArrayAdd");
			goto freeList;
		}
	}

	Sleep(2000);
	codeExit = 0;
freeList:
	if(!KHOPANArrayFree(&list)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANArrayInitialize");
		codeExit = 1;
	}

	Sleep(INFINITE);
	return codeExit;
}
