#include <Windows.h>

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	MessageBoxW(NULL, L"Hello, world!", L"Information", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	switch(reason) {
	case DLL_PROCESS_ATTACH:
		Execute(NULL, NULL, NULL, 0);
		return TRUE;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		return FALSE;
	}
}
