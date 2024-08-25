#include <stdio.h>
#include <khopanwin32.h>
#include "hrsp_client.h"

int main(int argc, char** argv) {
	HRSPCLIENTSTRUCT client = {0};
	HRSPCLIENTERROR error;

	if(!HRSPConnectToServer("localhost", "42485", &client, &error)) {
		if(error.remoteError) {
			printf("Custom Error: %d\n", error.codeRemote);
			return 1;
		}

		KHWin32ConsoleErrorW(error.code, error.function);
		return 1;
	}

	return 0;
}
