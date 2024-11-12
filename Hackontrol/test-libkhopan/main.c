#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error = {0};
	error.facility = ERROR_FACILITY_CURL;
	error.code = 25;
	error.source = L"CURLTest";
	//LPWSTR message = KHOPANGetErrorMessage(&error, NULL);
	//printf("Message: %ws\n", message);

	//if(message) {
	//	KHOPAN_DEALLOCATE(message);
	//}
	KHOPANERRORMESSAGE_KHOPAN(error);
	//KHOPANERRORCONSOLE_WIN32(ERROR_ABANDON_HIBERFILE, L"TriggerAbandonHiberfile");
	return 0;
}
