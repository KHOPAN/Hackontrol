#include <libkhopan.h>

int main(int argc, char** argv) {
	LPWSTR message = KHOPANFormatMessage(L"%ws:%02d:%S", L"12", 0, "00");
	printf("Message: %ws\n", message);

	if(message) {
		KHOPAN_DEALLOCATE(message);
	}

	return 0;
}
