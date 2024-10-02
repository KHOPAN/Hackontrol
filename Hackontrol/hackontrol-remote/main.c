#include <libkhopanlist.h>
#include "remote.h"

LINKEDLIST clientList;
HANDLE clientListMutex;

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int options) {
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

	if(!SetConsoleTitleW(L"Remote Console")) {
		KHOPANLASTERRORMESSAGE_WIN32(L"SetConsoleTitleW");
		goto functionExit;
	}
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

	SOCKET socketListen = 0;
	HANDLE thread = CreateThread(NULL, 0, ThreadServer, &socketListen, 0, NULL);

	if(!thread) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateThread");
		goto closeClientListMutex;
	}

	codeExit = WindowMain(instance);

	if(socketListen) {
		closesocket(socketListen);
	}

	if(WaitForSingleObject(thread, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORMESSAGE_WIN32(L"WaitForSingleObject");
		codeExit = 1;
	}

	CloseHandle(thread);
closeClientListMutex:
	if(!WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORMESSAGE_WIN32(L"WaitForSingleObject");
		codeExit = 1;
	}

	CloseHandle(clientListMutex);
freeClientList:
	if(!KHOPANLinkedFree(&clientList)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedFree");
		codeExit = 1;
	}
cleanupSocket:
	if(WSACleanup() == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"WSACleanup");
		codeExit = 1;
	}
functionExit:
	LOG("[Remote]: Exit with code: %d\n", codeExit);
#ifdef LOGGER_ENABLE
	Sleep(INFINITE);
#endif
	return codeExit;
}
