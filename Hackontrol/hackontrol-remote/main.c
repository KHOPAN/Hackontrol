#include <hackontrolpacket.h>
#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanarray.h>
#include "thread_server.h"
#include "thread_client.h"
#include "thread_window.h"
#include "window_main.h"
#include "logger.h"

HINSTANCE programInstance;
ArrayList clientList;
HANDLE listMutex;

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
	programInstance = instance;
	int returnValue = 1;
#ifdef LOGGER_ENABLE
#ifndef NO_CONSOLE
	if(!AllocConsole()) {
		KHWin32DialogErrorW(GetLastError(), L"AllocConsole");
		goto exit;
	}

	FILE* file = stdout;
	freopen_s(&file, "CONOUT$", "w", stdout);
	file = stderr;
	freopen_s(&file, "CONOUT$", "w", stderr);
	SetWindowTextW(GetConsoleWindow(), L"Hackontrol Remote Debug Log");
#endif
#endif
	LOG("[Hackontrol Remote]: Initializing Hackontrol Remote\n");

	if(!InitializeMainWindow()) {
		goto exit;
	}

	if(!WindowRegisterClass()) {
		goto exit;
	}

	if(!KHArrayInitialize(&clientList, sizeof(CLIENT))) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayInitialize");
		goto exit;
	}

	listMutex = CreateMutexExW(NULL, NULL, 0, DELETE | SYNCHRONIZE);

	if(!listMutex) {
		KHWin32DialogErrorW(GetLastError(), L"CreateMutexExW");
		goto freeClientList;
	}

	LOG("[Hackontrol Remote]: Starting server thread\n");
	HANDLE serverThread = CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);

	if(!serverThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		goto closeMutex;
	}

	returnValue = MainWindowMessageLoop();
	ExitServerThread();
	LOG("[Hackontrol Remote]: Waiting for all client threads to exit\n");

	for(size_t i = 0; i < clientList.elementCount; i++) {
		PCLIENT client;

		if(!KHArrayGet(&clientList, i, &client)) {
			KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
			continue;
		}

		if(!client) {
			continue;
		}

		closesocket(client->socket);

		if(client->thread) {
			WaitForSingleObject(client->thread, INFINITE);
		}
	}

	LOG("[Hackontrol Remote]: Waiting for server thread to exit\n");
	WaitForSingleObject(serverThread, INFINITE);
	CloseHandle(serverThread);
closeMutex:
	CloseHandle(listMutex);
freeClientList:
	KHArrayFree(&clientList);
exit:
	LOG("[Hackontrol Remote]: Exiting the main thread (Exit code: %d)\n" COMMA returnValue);
	return returnValue;
}
