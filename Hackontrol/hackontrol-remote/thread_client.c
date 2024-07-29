#include "thread_client.h"
#include <khopanwin32.h>
#include <khopanarray.h>
#include "window_main.h"
#include "logger.h"

extern ArrayList clientList;

DWORD WINAPI ClientThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client Thread]: Exiting with an error: No client structure provided\n");
		return 1;
	}

	LOG("[Client Thread %ws]: Hello from client thread\n" COMMA client->address);
	int returnValue = 1;

	if(!KHArrayAdd(&clientList, client)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		goto exit;
	}

	RefreshMainWindowListView();
	Sleep(5000);

	for(size_t i = 0; i < clientList.elementCount; i++) {
		PCLIENT instance;

		if(!KHArrayGet(&clientList, i, &instance)) {
			KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
			break;
		}

		if(instance->socket != client->socket) {
			continue;
		}

		if(!KHArrayRemove(&clientList, i)) {
			KHWin32DialogErrorW(GetLastError(), L"KHArrayRemove");
			break;
		}

		goto found;
	}

	LOG("[Client Thread]: Error: Client not found in the client list\n");
found:
	returnValue = 0;
	closesocket(client->socket);
	CloseHandle(client->thread);
exit:
	LOG("[Client Thread %ws]: Exiting the client thread (Exit code: %d)\n" COMMA client->address COMMA returnValue);
	LocalFree(client);
	return returnValue;
}
