#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error;
	/*LPWSTR text = KHOPANFileGetRundll32(&error);

	if(!text) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	printf("Rundll32: %ws\n", text);
	KHOPAN_DEALLOCATE(text);*/

	if(!KHOPANExecuteRundll32Function(L"D:\\GitHub Repository\\Hackontrol\\release\\GPURender.dll", "Execute", NULL, FALSE, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	printf("Hello, world!\n");
	Sleep(INFINITE);
	return 0;
}
