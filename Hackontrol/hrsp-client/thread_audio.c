#include <stdio.h>
#include "hrsp_client_internal.h"

DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		switch(message.message) {
		case AM_QUERY_AUDIO_DEVICE:
			break;
		}
	}

	return 0;
}
