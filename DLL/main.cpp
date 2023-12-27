#include <Windows.h>
#include "definition.h"

EXPORT Download(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	MessageBoxW(NULL, L"Hello", L"Hello, world!", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
