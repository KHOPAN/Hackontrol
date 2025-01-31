#include <hrsp_client.h>

/*static void __stdcall connected(const LPVOID parameter) {
	printf("Connected\n");
}*/

int main(int argc, char** argv) {
	/*HRSPCLIENTINPUT input = {0};
	input.callbackConnected = connected;
	KHOPANERROR error;

	if(!HRSPClientConnectToServer(NULL, NULL, &input, &error)) {
		KHOPANERRORCONSOLE_KHOPAN(error);
		return 1;
	}*/

	HRPSCLIENTPARAMETER parameter = {0};
	HRSPCLIENTSTATUS status = HRSPClientConnect(&parameter);
	printf("Status: %d\n", status);
	return 0;
}
