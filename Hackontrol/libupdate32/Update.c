#include <khopanwin32.h>
#include <khopanstring.h>

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	HANDLE file = CreateFileW(L"D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Debug\\libupdate32.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFileW");
		return;
	}

	LARGE_INTEGER fileSize;

	if(!GetFileSizeEx(file, &fileSize)) {
		KHWin32DialogErrorW(GetLastError(), L"GetFileSizeEx");
		goto closeFile;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, fileSize.QuadPart);

	if(!buffer) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		goto closeFile;
	}

	DWORD bytesRead;

	if(!ReadFile(file, buffer, fileSize.LowPart, &bytesRead, NULL)) {
		KHWin32DialogErrorW(GetLastError(), L"ReadFile");
		goto freeBuffer;
	}

	if(fileSize.LowPart != bytesRead) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"ReadFile");
		goto freeBuffer;
	}

freeBuffer:
	if(LocalFree(buffer)) {
		KHWin32DialogErrorW(GetLastError(), L"LocalFree");
	}
closeFile:
	if(!CloseHandle(file)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
	}
}
