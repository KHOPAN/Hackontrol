#include <stdio.h>
#include "hrsp_client_internal.h"

DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	printf("Audio Thread Finished\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		printf("Message: %u\n", message.message);
		Sleep(1000);
		printf("Audio Wait Done\n");
	}

	return 0;
}
