#include "hrsp_client_internal.h"

DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	return 0;
}
