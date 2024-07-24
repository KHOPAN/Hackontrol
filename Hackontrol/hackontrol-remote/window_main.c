#include "window.h"

DWORD WINAPI MainWindowThread(_In_ LPVOID parameter) {
	MessageBoxW(NULL, L"Main Thread", L"Hackontrol Remote", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return 0;
}
