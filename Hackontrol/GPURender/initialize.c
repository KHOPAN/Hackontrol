#include "main.h"

#ifdef DLL

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	InitializeGPURender(NULL);
}

#else

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR arguments, _In_ int commandShow) {
	return InitializeGPURender(instance);
}

#endif
