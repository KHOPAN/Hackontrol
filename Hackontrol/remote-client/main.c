#include <stdio.h>
#include <hrsp_client.h>

static void __stdcall connected(LPVOID parameter) {
	printf("Connected\n");
}

int main(int argc, char** argv) {
	HRSPCLIENTINPUT input = {0};
	input.callbackConnected = connected;
	HRSPCLIENTERROR error;

	if(!HRSPClientConnectToServer(NULL, NULL, &input, &error)) {
		LPWSTR message = HRSPClientGetErrorMessage(L"HRSPClientConnectToServer", &error);

		if(message) {
			printf("%ws\n", message);
			LocalFree(message);
		}

		return 1;
	}

	return 0;
}
