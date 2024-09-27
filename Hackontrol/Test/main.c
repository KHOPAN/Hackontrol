#include <libkhopan.h>
#include <libkhopanlist.h>

int main(int argc, char** argv) {
	DATASTREAM stream;

	if(!KHOPANStreamInitialize(&stream, 10)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANStreamInitialize");
		return 1;
	}

	int codeExit = 1;
	ULONGLONG data = 0x4141414141414141;

	if(!KHOPANStreamAdd(&stream, (PBYTE) &data, sizeof(ULONGLONG))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANStreamAdd");
		goto freeStream;
	}

	data = 0x4242424242424242;

	if(!KHOPANStreamAdd(&stream, (PBYTE) &data, sizeof(ULONGLONG))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANStreamAdd");
		goto freeStream;
	}

	codeExit = 0;
freeStream:
	if(!KHOPANStreamFree(&stream)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANStreamFree");
		codeExit = 1;
	}

	Sleep(INFINITE);
	return codeExit;
}
