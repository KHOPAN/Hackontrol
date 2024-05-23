#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	HANDLE file = CreateFileW(L"%SystemRoot%\\System32\\rundll32.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32ConsoleErrorW(GetLastError(), L"CreateFileW");
		return 1;
	}

closeFile:
	if(!CloseHandle(file)) {
		KHWin32ConsoleErrorW(GetLastError(), L"CloseHandle");
	}

	return 0;
}
