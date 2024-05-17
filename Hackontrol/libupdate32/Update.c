#include <khopanwin32.h>
#include <khopanstring.h>

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	DWORD processIdentifier = (DWORD) atol(argument);
	LPWSTR message = KHFormatMessageW(L"Process identifier: %lu", processIdentifier);

	if(!message) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	MessageBoxW(NULL, message, L"Argument", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
