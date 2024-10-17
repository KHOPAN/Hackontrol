#include <libkhopanlist.h>
#include "remote.h"
#include <CommCtrl.h>

HINSTANCE instance;
HANDLE clientListMutex;
LINKEDLIST clientList;
HFONT font;

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
	clientListMutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!clientListMutex) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateMutexExW");
		goto functionExit;
	}

	if(!KHOPANLinkedInitialize(&clientList, sizeof(CLIENT))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedInitialize");
		goto closeClientListMutex;
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"SystemParametersInfoW");
		goto freeClientList;
	}

	font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateFontIndirectW");
		goto freeClientList;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_LISTVIEW_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto deleteFont;
	}

	controls.dwICC = ICC_TAB_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto deleteFont;
	}

	if(!WindowSessionInitialize()) {
		goto deleteFont;
	}

	if(!WindowStreamInitialize()) {
		goto cleanupSession;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		KHOPANERRORMESSAGE_WIN32(status, L"WSAStartup");
		goto unregisterStream;
	}

	SOCKET socketListen = 0;
	HANDLE thread = CreateThread(NULL, 0, ThreadServer, &socketListen, 0, NULL);

	if(!thread) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateThread");
		goto cleanupSocket;
	}

	codeExit = WindowMain();

	if(socketListen) {
		closesocket(socketListen);
		socketListen = 0;
	}

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
cleanupSocket:
	WSACleanup();
unregisterStream:
	UnregisterClassW(CLASS_SESSION_STREAM, instance);
cleanupSession:
	WindowSessionCleanup();
deleteFont:
	DeleteObject(font);
freeClientList:
	KHOPANLinkedFree(&clientList);
closeClientListMutex:
	WaitForSingleObject(clientListMutex, INFINITE);
	CloseHandle(clientListMutex);
functionExit:
	LOG("[Remote]: Exit with code: %d\n", codeExit);
#ifdef LOGGER_ENABLE
	Sleep(INFINITE);
#endif
	return codeExit;
}
