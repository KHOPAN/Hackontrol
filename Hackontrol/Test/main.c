#include <libkhopan.h>
#include <libkhopanlist.h>

int main(int argc, char** argv) {
	LINKEDLIST list;

	if(!KHOPANLinkedInitialize(&list, sizeof(ULONGLONG))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedInitialize");
		return 1;
	}

	int codeExit = 1;

	for(ULONGLONG i = 1; i <= 10; i++) {
		if(!KHOPANLinkedAdd(&list, (PBYTE) &i, NULL)) {
			KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedAdd");
			goto freeList;
		}
	}

	for(PLINKEDLISTITEM item = list.item; item; item = item->next) {
		printf("Number: %llu\n", *((PULONGLONG) item->data));
	}

	codeExit = 0;
freeList:
	if(!KHOPANLinkedFree(&list)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedFree");
		codeExit = 1;
	}

	return codeExit;
}
