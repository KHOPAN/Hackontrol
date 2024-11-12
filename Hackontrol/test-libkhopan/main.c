#include <libkhopan.h>

int main(int argc, char** argv) {
	/*KHOPANERROR error;

	if(!KHOPANExecuteCommand(L"start chrome https://www.youtube.com", TRUE, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}*/

	LPWSTR text = KHOPANFileGetCmd();
	printf("%ws\n", text);

	if(text) {
		KHOPAN_DEALLOCATE(text);
	}

	return 0;
}
