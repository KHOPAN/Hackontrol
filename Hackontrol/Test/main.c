#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error;
	LPWSTR text = KHOPANFileGetRundll32(&error);

	if(!text) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	printf("Rundll32: %ws\n", text);
	KHOPAN_DEALLOCATE(text);
	return 0;
}
