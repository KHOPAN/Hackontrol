#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error = {0};
	error.facility = ERROR_FACILITY_COMMON;
	error.code = ERROR_COMMON_INDEX_OUT_OF_BOUNDS;
	LPCWSTR message = KHOPANErrorCommonDecoder(&error);
	printf("Message: %ws\n", message);
	return 0;
}
