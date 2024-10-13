#include <libkhopanlist.h>
#include "remote.h"

LINKEDLIST clientList;
HANDLE clientListMutex;
HINSTANCE instance;

int WINAPI WinMain(_In_ HINSTANCE programInstance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int options) {
	instance = programInstance;
	int codeExit = 1;
#if defined(LOGGER_ENABLE) && !defined(NO_CONSOLE)
	if(!AllocConsole()) {
		KHOPANLASTERRORMESSAGE_WIN32(L"AllocConsole");
		goto functionExit;
	}

	FILE* file = stdout;
	freopen_s(&file, "CONOUT$", "w", stdout);
	file = stderr;
	freopen_s(&file, "CONOUT$", "w", stderr);
	SetConsoleTitleW(L"Remote Console");
#endif
	LOG("[Remote]: Initializing\n");
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		KHOPANERRORMESSAGE_WIN32(status, L"WSAStartup");
		goto functionExit;
	}

	if(!KHOPANLinkedInitialize(&clientList, sizeof(CLIENT))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedInitialize");
		goto cleanupSocket;
	}

	clientListMutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!clientListMutex) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateMutexExW");
		goto freeClientList;
	}

	if(!WindowSessionInitialize()) {
		goto closeClientListMutex;
	}

	if(!WindowStreamInitialize()) {
		goto unregisterSession;
	}

	SOCKET socketListen = 0;
	HANDLE thread = CreateThread(NULL, 0, ThreadServer, &socketListen, 0, NULL);

	if(!thread) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateThread");
		goto unregisterStream;
	}

	codeExit = WindowMain();

	if(socketListen) {
		closesocket(socketListen);
		socketListen = 0;
	}

	REMOTE_CLOSE_HANDLE(thread);
unregisterStream:
	UnregisterClassW(CLASS_SESSION_STREAM, instance);
unregisterSession:
	UnregisterClassW(CLASS_REMOTE_SESSION, instance);
closeClientListMutex:
	REMOTE_CLOSE_HANDLE(clientListMutex);
freeClientList:
	KHOPANLinkedFree(&clientList);
cleanupSocket:
	WSACleanup();
functionExit:
	LOG("[Remote]: Exit with code: %d\n", codeExit);
#ifdef LOGGER_ENABLE
	Sleep(INFINITE);
#endif
	return codeExit;
}
