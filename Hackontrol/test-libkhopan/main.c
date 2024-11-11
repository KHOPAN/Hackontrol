#include <libkhopan.h>

int main(int argc, char** argv) {
	LPSTR message = KHOPANFormatANSI("%ws:%02d", L"12", 0);
	printf("Message: %s\n", message);

	if(message) {
		KHOPAN_DEALLOCATE(message);
	}

	return 0;
}
