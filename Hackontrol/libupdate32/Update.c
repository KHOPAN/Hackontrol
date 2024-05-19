#include <khopanwin32.h>

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	DWORD processIdentifier = (DWORD) atol(argument);
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIdentifier);

	if(!process) {
		KHWin32DialogErrorW(GetLastError(), L"OpenProcess");
		return;
	}

	if(WaitForSingleObject(process, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		return;
	}

	if(!CloseHandle(process)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return;
	}

	MessageBoxW(NULL, L"The process has exited", L"libupdate32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
