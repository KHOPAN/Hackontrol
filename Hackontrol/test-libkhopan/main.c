#include <libkhopanlist.h>

int streamTest() {
	DATASTREAM stream;
	KHOPANERROR error;

	if(!KHOPANStreamInitialize(&stream, 0, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	BYTE data[64];
	memset(data, 5, sizeof(data));
	int codeExit = 1;

	for(UINT i = 0; i < 100; i++) {
		if(!KHOPANStreamAdd(&stream, data, sizeof(data), &error)) {
			KHOPANERRORMESSAGE_KHOPAN(error);
			goto freeStream;
		}
	}

	codeExit = 0;
freeStream:
	if(!KHOPANStreamFree(&stream, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		codeExit = 1;
	}

	return codeExit;
}

int arrayTest() {
	ARRAYLIST list;
	KHOPANERROR error;
	BYTE data[16];
	memset(data, 5, sizeof(data));

	if(!KHOPANArrayInitialize(&list, sizeof(data), &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	int codeExit = 0;

	for(UINT i = 0; i < 100; i++) {
		if(!KHOPANArrayAdd(&list, data, &error)) {
			KHOPANERRORMESSAGE_KHOPAN(error);
			goto freeStream;
		}
	}

	if(!KHOPANArrayRemove(&list, 5, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		goto freeStream;
	}

	codeExit = 0;
freeStream:
	if(!KHOPANArrayFree(&list, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		codeExit = 1;
	}

	return codeExit;
}

int linkedTest() {
	LINKEDLIST list;
	KHOPANERROR error;

	if(!KHOPANLinkedInitialize(&list, sizeof(ULONGLONG), &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	int codeExit = 0;

	for(UINT i = 0; i < 15; i++) {
		ULONGLONG number = i;

		if(!KHOPANLinkedAdd(&list, (PBYTE) &number, NULL, &error)) {
			KHOPANERRORMESSAGE_KHOPAN(error);
			goto freeStream;
		}
	}

	PLINKEDLISTITEM item;

	if(!KHOPANLinkedGet(&list, 5, &item, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		goto freeStream;
	}

	if(!KHOPANLinkedRemove(item, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		goto freeStream;
	}

	KHOPAN_LINKED_LIST_ITERATE_FORWARD(item, &list) {
		printf("%llu\n", *((PULONGLONG) item->data));
	}

	codeExit = 0;
freeStream:
	if(!KHOPANLinkedFree(&list, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		codeExit = 1;
	}

	return codeExit;
}

int main(int argc, char** argv) {
	return linkedTest();
}
