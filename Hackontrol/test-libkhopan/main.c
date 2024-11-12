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

int main(int argc, char** argv) {
	return arrayTest();
}
