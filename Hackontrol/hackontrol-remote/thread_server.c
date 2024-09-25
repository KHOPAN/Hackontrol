#include "logger.h"

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int options) {
	int codeExit = 1;
	FILE* file;
#if defined(LOGGER_ENABLE) && !defined(NO_CONSOLE)
	if(!AllocConsole()) {
		KHOPANLASTERRORMESSAGE_WIN32(L"AllocConsole");
		goto functionExit;
	}

	file = stdout;
	freopen_s(&file, "CONOUT$", "w", stdout);
	file = stderr;
	freopen_s(&file, "CONOUT$", "w", stderr);

	if(!SetConsoleTitleW(L"Remote Console")) {
		KHOPANLASTERRORMESSAGE_WIN32(L"SetConsoleTitleW");
		goto functionExit;
	}
#endif
	LOG("[Remote]: Initializing\n");
functionExit:
	LOG("[Remote]: Exit with code: %d\n", codeExit);
#ifdef LOGGER_ENABLE
	Sleep(INFINITE);
#endif
	return codeExit;
}
