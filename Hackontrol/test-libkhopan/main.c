#include <libkhopan.h>

int main(int argc, char** argv) {
	KHOPANERROR error;

	if(!KHOPANExecuteProcess(L"C:\\Windows\\System32\\rundll32.exe", L"rundll32.exe \"D:\\GitHub Repository\\Hackontrol\\release\\GPURender.dll\" Execute", TRUE, &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		return 1;
	}

	return 0;
}
