#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERRORMESSAGE_WIN32(ERROR_ABANDON_HIBERFILE, L"TriggerAbandonHiberfile");
	return 0;
}
