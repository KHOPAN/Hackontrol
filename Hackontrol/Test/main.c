#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERRORMESSAGE_NTSTATUS(STATUS_ALREADY_REGISTERED, L"SimpleFunction");
	return 0;
}
