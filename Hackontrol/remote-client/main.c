#include <stdio.h>
#include <hrsp_client.h>

int main(int argc, char** argv) {
	HRSPCLIENTERROR error;

	if(!HRSPClientConnectToServer("localhost", "42485", &error)) {
		printf("Function: %ws Error code: %u\n", error.function, error.code);
		return 1;
	}

	return 0;
}
