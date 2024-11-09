#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error;

	if(!KHOPANEnablePrivilege(L"PleaseDoNotExist", &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	return 0;
}
