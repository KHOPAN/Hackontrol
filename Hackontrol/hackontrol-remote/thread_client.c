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
	returnValue = 0;
	closesocket(client->socket);
	CloseHandle(client->thread);
exit:
	LOG("[Client Thread %ws]: Exiting the client thread (Exit code: %d)\n" COMMA client->address COMMA returnValue);
	LocalFree(client);
	return returnValue;
}
