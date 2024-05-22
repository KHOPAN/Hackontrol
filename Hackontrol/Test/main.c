#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	if(!KHWin32StartProcessW(L"C:\\Windows\\System32\\cmd.exe", L"/c start chrome https://www.youtube.com")) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32StartProcessW");
		return 1;
	}

	return 0;
}
