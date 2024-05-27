#include "initialize.h"

static HINSTANCE globalProgramInstance;

BOOL WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved) {
	globalProgramInstance = instance;
	return TRUE;
}

HINSTANCE GetProgramInstance() {
	return globalProgramInstance;
}
