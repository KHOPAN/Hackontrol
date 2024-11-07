#include <libkhopan.h>

int main(int argc, char** argv) {
	//HRESULT result = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	LPWSTR message = KHOPANGetErrorMessageHRESULT(STATUS_ENTRYPOINT_NOT_FOUND | 0x10000000);
	printf("Message: %ws\n", message);

	if(message) {
		KHOPAN_DEALLOCATE(message);
	}

	return 0;
}
