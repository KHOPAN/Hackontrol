#include <libkhopanlist.h>

int main(int argc, char** argv) {
	DATASTREAM stream;
	KHOPANERROR error;

	if(!KHOPANStreamInitialize(&stream, 0, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	if(!KHOPANStreamAdd(&stream, (PBYTE) "Four", 4, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	if(!KHOPANStreamFree(&stream, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	return 0;
}
