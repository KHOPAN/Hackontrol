#include <stdio.h>
#include <hrsp_client.h>

int main(int argc, char** argv) {
	HRSPCLIENTERROR error;

	if(!HRSPClientConnectToServer(NULL, NULL, &error)) {
		LPWSTR message = HRSPClientGetErrorMessage(L"HRSPClientConnectToServer", &error);

		if(message) {
			printf("%ws\n", message);
			LocalFree(message);
		}

		return 1;
	}

	return 0;
}
