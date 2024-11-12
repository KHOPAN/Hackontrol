#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error;

	if(!KHOPANExecuteRundll32Function(L"D:\\GitHub Repository\\Hackontrol\\release\\GPURender.dll", "Execute", NULL, TRUE, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	return 0;
}
