#include <libkhopan.h>

int main(int argc, char** argv) {
	if(WaitForSingleObject(NULL, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORMESSAGE_WIN32(L"WaitForSingleObject");
		return 1;
	}

	return 0;
}
