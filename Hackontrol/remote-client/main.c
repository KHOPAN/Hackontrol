#include <stdio.h>
#include <hrsp_client.h>

static void __stdcall connected(LPVOID parameter) {
	printf("Connected\n");
}

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Username is missing!\n");
		return 1;
	}

	HRSPCLIENTINPUT input = {0};
	input.callbackConnected = connected;
	HRSPCLIENTERROR error;

	if(!HRSPClientConnectToServer(NULL, NULL, &input, &error, argv[1])) {
		LPWSTR message = HRSPClientGetErrorMessage(L"HRSPClientConnectToServer", &error);

		if(message) {
			printf("%ws\n", message);
			LocalFree(message);
		}

		return 1;
	}

	return 0;
}
