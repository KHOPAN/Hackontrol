#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error = {0};
	error.facility = ERROR_FACILITY_COMMON;
	error.code = ERROR_COMMON_INVALID_PARAMETER;
	LPWSTR message = KHOPANGetErrorMessage(&error);
	printf("%ws\n", message);

	if(message) {
		KHOPAN_DEALLOCATE(message);
	}

	return 0;
}
