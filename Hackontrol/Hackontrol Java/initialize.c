#include "initialize.h"

static HINSTANCE globalInstance;

HINSTANCE GetProgramInstance() {
	return globalInstance;
}

BOOL WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved) {
	globalInstance = instance;
	return TRUE;
}
