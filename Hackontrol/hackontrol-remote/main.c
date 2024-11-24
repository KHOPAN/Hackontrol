#include "remote.h"
#include <CommCtrl.h>

HINSTANCE instance;
HFONT font;
LINKEDLIST clientList;
HANDLE clientListMutex;

static DWORD controlList[] = {
	ICC_LISTVIEW_CLASSES,
	ICC_TAB_CLASSES
};

int WINAPI WinMain(_In_ HINSTANCE programInstance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int options) {
	instance = programInstance;
	int codeExit = 1;
#ifdef LOGGER_ENABLE
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
	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);

	for(size_t i = 0; i < sizeof(controlList) / sizeof(controlList[0]); i++) {
		controls.dwICC = controlList[i];

		if(!InitCommonControlsEx(&controls)) {
			KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
			goto functionExit;
		}
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"SystemParametersInfoW");
		goto functionExit;
	}

	font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateFontIndirectW");
		goto functionExit;
	}

	KHOPANERROR error;

	if(!KHOPANLinkedInitialize(&clientList, sizeof(CLIENT), &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		goto deleteFont;
	}

	clientListMutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!clientListMutex) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateMutexExW");
		goto freeClientList;
	}

	if(!WindowMainInitialize()) {
		goto closeClientListMutex;
	}

	if(!WindowSessionInitialize()) {
		goto destroyMainWindow;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		KHOPANERRORMESSAGE_WIN32(status, L"WSAStartup");
		goto cleanupSession;
	}

	SOCKET socketListen = INVALID_SOCKET;
	HANDLE serverThread = CreateThread(NULL, 0, ThreadServer, &socketListen, 0, NULL);

	if(!serverThread) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateThread");
		goto cleanupSocket;
	}

	WindowMain();

	if(socketListen != INVALID_SOCKET) {
		closesocket(socketListen);
		socketListen = INVALID_SOCKET;
	}

	WaitForSingleObject(serverThread, INFINITE);
	CloseHandle(serverThread);
	codeExit = 0;
cleanupSocket:
	WSACleanup();
cleanupSession:
	WindowSessionCleanup();
destroyMainWindow:
	WindowMainDestroy();
closeClientListMutex:
	WaitForSingleObject(clientListMutex, INFINITE);
	CloseHandle(clientListMutex);
freeClientList:
	KHOPANLinkedFree(&clientList, NULL);
deleteFont:
	DeleteObject(font);
functionExit:
	LOG("[Remote]: Exit with code: %d\n", codeExit);
#ifdef LOGGER_ENABLE
	Sleep(INFINITE);
#endif
	return codeExit;
}
