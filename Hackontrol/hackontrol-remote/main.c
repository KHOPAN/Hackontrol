#include "connection.h"
#include <hackontrolpacket.h>
#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanarray.h>
#include "window_main.h"
#include "logger.h"

#define IDM_REMOTE_ALWAYS_ON_TOP 0xE000
#define IDM_REMOTE_OPEN          0xE001
#define IDM_REMOTE_DISCONNECT    0xE002

#pragma warning(disable: 6001)
#pragma warning(disable: 6258)

static ArrayList globalClientList;
static int globalExitCode;
static HWND globalWindow;
static HWND globalTitledBorder;
static HWND globalListView;
static HMENU globalPopupMenu;

/*static LRESULT CALLBACK hackontrolRemoteProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(globalWindow);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(globalExitCode);
		return 0;
	case WM_SIZE: {
		RECT bounds;
		GetClientRect(globalWindow, &bounds);
		SetWindowPos(globalTitledBorder, HWND_TOP, 0, 0, bounds.right - bounds.left - 10, bounds.bottom - bounds.top - 4, SWP_NOMOVE);
		GetClientRect(globalTitledBorder, &bounds);
		SetWindowPos(globalListView, HWND_TOP, bounds.left + 9, bounds.top + 17, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 22, 0);
		return 0;
	}
	case WM_CONTEXTMENU: {
		int x = LOWORD(lparam);
		int y = HIWORD(lparam);
		LVHITTESTINFO hitTest = {0};
		hitTest.pt.x = x;
		hitTest.pt.y = y;
		ScreenToClient(globalListView, &hitTest.pt);

		if(SendMessageW(globalListView, LVM_HITTEST, 0, (LPARAM) &hitTest) == -1) {
			return 0;
		}

		CLIENTENTRY* entry;

		if(!KHArrayGet(&globalClientList, hitTest.iItem, &entry)) {
			return 0;
		}

		SetForegroundWindow(globalWindow);
		PACKET packet = {0};

		switch(TrackPopupMenuEx(globalPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, x, y, window, NULL)) {
		case IDM_REMOTE_DISCONNECT:
			packet.packetType = PACKET_TYPE_INFORMATION;
			SendPacket(entry->clientSocket, &packet);
			RemoteRemoveEntry(entry->clientSocket);
			RemoteRefreshClientList();
			break;
		}

		return 0;
	}
	case WM_SYSCOMMAND:
		if(LOWORD(wparam) != IDM_REMOTE_ALWAYS_ON_TOP) {
			break;
		}

		SetWindowPos(globalWindow, (GetWindowLongW(globalWindow, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		return TRUE;
	case WM_WINDOWPOSCHANGED: {
		WINDOWPOS* position = (WINDOWPOS*) lparam;

		if(!position || position->flags & SWP_NOZORDER) {
			break;
		}

		CheckMenuItem(GetSystemMenu(globalWindow, FALSE), IDM_REMOTE_ALWAYS_ON_TOP, MF_BYCOMMAND | (GetWindowLongW(globalWindow, GWL_EXSTYLE) & WS_EX_TOPMOST ? MF_CHECKED : MF_UNCHECKED));
		break;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}*/

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
#ifdef LOGGER_ENABLE
	if(!AllocConsole()) {
		KHWin32DialogErrorW(GetLastError(), L"AllocConsole");
		return 1;
	}

	FILE* standardOutput = stdout;
	FILE* standardError = stderr;
	freopen_s(&standardOutput, "CONOUT$", "w", stdout);
	freopen_s(&standardError, "CONOUT$", "w", stderr);
	SetWindowTextW(GetConsoleWindow(), L"Hackontrol Remote Debug Log");
#endif
	LOG("[Hackontrol Remote]: Initializing Hackontrol Remote\n");

	if(!InitializeMainWindow(instance)) {
		return 1;
	}

	LOG("[Hackontrol Remote]: Starting server thread\n");
	HANDLE serverThread = CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);

	if(!serverThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		return 1;
	}

	CloseHandle(serverThread);
	return MainWindowMessageLoop();
	/*int returnValue = 1;

	if(!KHArrayInitialize(&globalClientList, sizeof(CLIENTENTRY))) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayInitialize");
		goto unregisterWindowClass;
	}

	HMENU systemMenu = GetSystemMenu(globalWindow, FALSE);

	if(!InsertMenuW(systemMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED | MF_STRING | (GetWindowLongW(globalWindow, GWL_EXSTYLE) & WS_EX_TOPMOST ? MF_CHECKED : MF_UNCHECKED), IDM_REMOTE_ALWAYS_ON_TOP, L"Always On Top")) {
		RemoteError(GetLastError(), L"InsertMenuW");
		goto closeServerThread;
	}

	if(!InsertMenuW(systemMenu, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL)) {
		RemoteError(GetLastError(), L"InsertMenuW");
		goto closeServerThread;
	}

	globalPopupMenu = CreatePopupMenu();

	if(!globalPopupMenu) {
		RemoteError(GetLastError(), L"CreatePopupMenu");
		goto deleteFont;
	}

	if(!InsertMenuW(globalPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDM_REMOTE_OPEN, L"Open")) {
		RemoteError(GetLastError(), L"InsertMenuW");
		goto destroyMenu;
	}

	if(!InsertMenuW(globalPopupMenu, 1, MF_BYPOSITION | MF_STRING, IDM_REMOTE_DISCONNECT, L"Disconnect")) {
		RemoteError(GetLastError(), L"InsertMenuW");
		goto destroyMenu;
	}

	returnValue |= globalExitCode;
destroyMenu:
	DestroyMenu(globalPopupMenu);
closeServerThread:
	TerminateThread(serverThread, 0);
	CloseHandle(serverThread);
freeGlobalClientList:
	for(size_t i = 0; i < globalClientList.elementCount; i++) {
		CLIENTENTRY* entry;

		if(!KHArrayGet(&globalClientList, i, &entry)) {
			continue;
		}

		if(entry->username) {
			LocalFree(entry->username);
		}

		LocalFree(entry->address);

		if(entry->clientThread) {
			TerminateThread(entry->clientThread, 0);
			CloseHandle(entry->clientThread);
		}
	}

	KHArrayFree(&globalClientList);*/
	return 0;
}

void ExitRemote(int exitCode) {
	globalExitCode = exitCode;
	SendMessageW(globalWindow, WM_CLOSE, 0, 0);
}

void RemoteError(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWin32GetErrorMessageW(errorCode, functionName);
	MessageBoxW(globalWindow, message, L"Hackontrol Remote Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
	ExitRemote(errorCode);
}

void RemoteHandleConnection(const SOCKET clientSocket, LPWSTR address) {
	CLIENTENTRY entry = {0};
	entry.clientSocket = clientSocket;

	if(!KHArrayAdd(&globalClientList, &entry)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		closesocket(clientSocket);
		LocalFree(address);
		return;
	}

	CLIENTENTRY* entryPointer;

	if(!KHArrayGet(&globalClientList, globalClientList.elementCount - 1, &entryPointer)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
		RemoteRemoveEntry(clientSocket);
		closesocket(clientSocket);
		LocalFree(address);
		return;
	}

	entryPointer->address = address;
	HANDLE clientThread = CreateThread(NULL, 0, ClientThread, entryPointer, 0, NULL);

	if(!clientThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		RemoteRemoveEntry(clientSocket);
		closesocket(clientSocket);
		LocalFree(address);
		return;
	}

	entryPointer->clientThread = clientThread;
}

void RemoteRemoveEntry(const SOCKET clientSocket) {
	for(size_t i = 0; i < globalClientList.elementCount; i++) {
		CLIENTENTRY* element;

		if(KHArrayGet(&globalClientList, i, &element) && element->clientSocket == clientSocket) {
			KHArrayRemove(&globalClientList, i);
			return;
		}
	}
}

void RemoteRefreshClientList() {
	/*SendMessageW(globalListView, LVM_DELETEALLITEMS, 0, 0);
	LVITEMW item = {0};
	item.mask = LVIF_TEXT;

	for(size_t i = 0; i < globalClientList.elementCount; i++) {
		CLIENTENTRY* entry;

		if(!KHArrayGet(&globalClientList, i, &entry) || !entry->username || !entry->address) {
			continue;
		}

		item.iSubItem = 0;
		item.pszText = entry->username;

		if(SendMessageW(globalListView, LVM_INSERTITEM, 0, (LPARAM) &item) == -1) {
			continue;
		}

		item.iSubItem = 1;
		item.pszText = entry->address;
		SendMessageW(globalListView, LVM_SETITEM, 0, (LPARAM) &item);
	}*/
}
