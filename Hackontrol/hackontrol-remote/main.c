#include <khopanwin32.h>

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
	WSADATA windowsSocketData;
	int status = WSAStartup(MAKEWORD(2, 2), &windowsSocketData);

	if(status) {
		KHWin32DialogErrorW(status, L"WSAStartup");
		return 1;
	}

	if(WSACleanup() == SOCKET_ERROR) {
		KHWin32DialogErrorW(status, L"WSACleanup");
		return 1;
	}

	return 0;
}
